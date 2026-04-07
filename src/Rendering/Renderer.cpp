#include "PixellentModeler/Rendering/Renderer.hpp"
#include "PixellentModeler/Rendering/Shader.hpp"
#include "PixellentModeler/Rendering/Texture.hpp"
#include "PixellentModeler/Mesh/MeshBuffer.hpp"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>

namespace PixellentModeler {

// ---------------------------------------------------------------------------
// Embedded shader sources
// ---------------------------------------------------------------------------

// ---- Basic shader: MVP + flat color ----
static const char* kBasicVertexSrc = R"glsl(
#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main() {
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
}
)glsl";

static const char* kBasicFragmentSrc = R"glsl(
#version 330 core
out vec4 FragColor;

uniform vec3 uColor;

void main() {
    FragColor = vec4(uColor, 1.0);
}
)glsl";

// ---- Blinn-Phong shader ----
static const char* kBlinnPhongVertexSrc = R"glsl(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat3 uNormalMatrix;

out vec3 vWorldPos;
out vec3 vNormal;
out vec2 vTexCoord;

void main() {
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    vWorldPos = worldPos.xyz;
    vNormal = normalize(uNormalMatrix * aNormal);
    vTexCoord = aTexCoord;
    gl_Position = uProjection * uView * worldPos;
}
)glsl";

static const char* kBlinnPhongFragmentSrc = R"glsl(
#version 330 core

#define MAX_LIGHTS 8

struct Light {
    int type;          // 0 = directional, 1 = point
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
};

uniform Light uLights[MAX_LIGHTS];
uniform int uNumLights;

uniform vec3 uAlbedo;
uniform float uMetallic;
uniform float uRoughness;
uniform vec3 uCameraPos;

uniform sampler2D uAlbedoMap;
uniform int uHasAlbedoMap;

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vTexCoord;

out vec4 FragColor;

void main() {
    vec3 albedo = uAlbedo;
    if (uHasAlbedoMap == 1) {
        albedo *= texture(uAlbedoMap, vTexCoord).rgb;
    }

    vec3 N = normalize(vNormal);
    vec3 V = normalize(uCameraPos - vWorldPos);

    // Ambient component
    vec3 ambient = 0.05 * albedo;
    vec3 result = ambient;

    for (int i = 0; i < uNumLights && i < MAX_LIGHTS; ++i) {
        vec3 L;
        float attenuation = 1.0;

        if (uLights[i].type == 0) {
            // Directional light
            L = normalize(-uLights[i].direction);
        } else {
            // Point light
            vec3 toLight = uLights[i].position - vWorldPos;
            float dist = length(toLight);
            L = normalize(toLight);
            attenuation = 1.0 / (uLights[i].constant + uLights[i].linear * dist +
                                  uLights[i].quadratic * dist * dist);
        }

        // Diffuse
        float diff = max(dot(N, L), 0.0);
        vec3 diffuse = diff * albedo * uLights[i].color * uLights[i].intensity;

        // Blinn-Phong specular
        vec3 H = normalize(L + V);
        float shininess = mix(8.0, 256.0, 1.0 - uRoughness);
        float spec = pow(max(dot(N, H), 0.0), shininess);
        vec3 specular = spec * uLights[i].color * uLights[i].intensity * mix(vec3(0.04), albedo, uMetallic);

        result += (diffuse + specular) * attenuation;
    }

    // Clamp to avoid oversaturation
    result = min(result, vec3(1.0));
    FragColor = vec4(result, 1.0);
}
)glsl";

// ---- Wireframe shader ----
static const char* kWireframeVertexSrc = R"glsl(
#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main() {
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
}
)glsl";

static const char* kWireframeFragmentSrc = R"glsl(
#version 330 core
out vec4 FragColor;

uniform vec3 uColor;

void main() {
    FragColor = vec4(uColor, 1.0);
}
)glsl";

// ---- Grid shader ----
static const char* kGridVertexSrc = R"glsl(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor;

uniform mat4 uView;
uniform mat4 uProjection;

out vec4 vColor;
out float vDist;

void main() {
    vColor = aColor;
    vec4 viewPos = uView * vec4(aPos, 1.0);
    vDist = length(viewPos.xyz);
    gl_Position = uProjection * viewPos;
}
)glsl";

static const char* kGridFragmentSrc = R"glsl(
#version 330 core
in vec4 vColor;
in float vDist;

out vec4 FragColor;

void main() {
    // Fade out grid lines at distance
    float fadeStart = 30.0;
    float fadeEnd = 50.0;
    float alpha = vColor.a * (1.0 - smoothstep(fadeStart, fadeEnd, vDist));
    if (alpha < 0.01) discard;
    FragColor = vec4(vColor.rgb, alpha);
}
)glsl";

// ---- Gizmo shader ----
static const char* kGizmoVertexSrc = R"glsl(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;

uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uModel;

out vec3 vColor;

void main() {
    vColor = aColor;
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
}
)glsl";

static const char* kGizmoFragmentSrc = R"glsl(
#version 330 core
in vec3 vColor;

out vec4 FragColor;

void main() {
    FragColor = vec4(vColor, 1.0);
}
)glsl";

// ---- Selection ID shader (GPU picking) ----
static const char* kSelectionIdVertexSrc = R"glsl(
#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main() {
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
}
)glsl";

static const char* kSelectionIdFragmentSrc = R"glsl(
#version 330 core
out int FragColor;

uniform int uObjectID;

void main() {
    FragColor = uObjectID;
}
)glsl";


// ---------------------------------------------------------------------------
// Renderer implementation
// ---------------------------------------------------------------------------

Renderer::Renderer() = default;

Renderer::~Renderer() {
    if (m_gridVBO) glDeleteBuffers(1, &m_gridVBO);
    if (m_gridVAO) glDeleteVertexArrays(1, &m_gridVAO);
    if (m_gizmoVBO) glDeleteBuffers(1, &m_gizmoVBO);
    if (m_gizmoVAO) glDeleteVertexArrays(1, &m_gizmoVAO);
}

bool Renderer::init(GLFWwindow* window) {
    m_window = window;
    if (!m_window) {
        std::cerr << "Renderer::init: window is null" << std::endl;
        return false;
    }

    // Load OpenGL function pointers via GLAD (safe to call multiple times)
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Renderer::init: failed to initialize GLAD" << std::endl;
        return false;
    }

    // Basic OpenGL state
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_MULTISAMPLE);

    // Set default viewport
    int w, h;
    glfwGetFramebufferSize(m_window, &w, &h);
    glViewport(0, 0, w, h);

    // Set up a default directional light
    LightData defaultLight;
    defaultLight.type = LightType::Directional;
    defaultLight.direction = glm::vec3(-0.3f, -1.0f, -0.5f);
    defaultLight.color = glm::vec3(1.0f, 1.0f, 1.0f);
    defaultLight.intensity = 1.0f;
    m_lights.push_back(defaultLight);

    initShaders();
    initGrid();
    initGizmo();

    m_initialized = true;
    std::cout << "Renderer initialized successfully" << std::endl;
    std::cout << "  OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "  Renderer: " << glGetString(GL_RENDERER) << std::endl;
    return true;
}

void Renderer::initShaders() {
    // Basic shader
    {
        auto shader = std::make_shared<Shader>();
        if (shader->create(kBasicVertexSrc, kBasicFragmentSrc)) {
            m_shaderLibrary.add("basic", std::move(shader));
        } else {
            std::cerr << "Failed to create 'basic' shader" << std::endl;
        }
    }

    // Blinn-Phong shader
    {
        auto shader = std::make_shared<Shader>();
        if (shader->create(kBlinnPhongVertexSrc, kBlinnPhongFragmentSrc)) {
            m_shaderLibrary.add("blinn_phong", std::move(shader));
        } else {
            std::cerr << "Failed to create 'blinn_phong' shader" << std::endl;
        }
    }

    // Wireframe shader
    {
        auto shader = std::make_shared<Shader>();
        if (shader->create(kWireframeVertexSrc, kWireframeFragmentSrc)) {
            m_shaderLibrary.add("wireframe", std::move(shader));
        } else {
            std::cerr << "Failed to create 'wireframe' shader" << std::endl;
        }
    }

    // Grid shader
    {
        auto shader = std::make_shared<Shader>();
        if (shader->create(kGridVertexSrc, kGridFragmentSrc)) {
            m_shaderLibrary.add("grid", std::move(shader));
        } else {
            std::cerr << "Failed to create 'grid' shader" << std::endl;
        }
    }

    // Gizmo shader
    {
        auto shader = std::make_shared<Shader>();
        if (shader->create(kGizmoVertexSrc, kGizmoFragmentSrc)) {
            m_shaderLibrary.add("gizmo", std::move(shader));
        } else {
            std::cerr << "Failed to create 'gizmo' shader" << std::endl;
        }
    }

    // Selection ID shader (GPU picking)
    {
        auto shader = std::make_shared<Shader>();
        if (shader->create(kSelectionIdVertexSrc, kSelectionIdFragmentSrc)) {
            m_shaderLibrary.add("selection_id", std::move(shader));
        } else {
            std::cerr << "Failed to create 'selection_id' shader" << std::endl;
        }
    }
}

void Renderer::initGrid() {
    // Generate grid lines from -50 to +50 in 1-unit steps along X and Z
    // Each vertex: position(3) + color(4)
    const int halfExtent = 50;
    const float lineAlpha = 0.3f;
    const float axisAlpha = 1.0f;

    std::vector<float> vertices;

    // Regular grid lines (grey)
    for (int i = -halfExtent; i <= halfExtent; ++i) {
        float fi = static_cast<float>(i);

        // Skip axis lines (we draw them separately with color)
        if (i == 0) continue;

        float brightness = (i % 10 == 0) ? 0.5f : 0.3f;
        float alpha = (i % 10 == 0) ? lineAlpha * 1.5f : lineAlpha;

        // Line along Z (X = i)
        vertices.insert(vertices.end(), {fi, 0.0f, static_cast<float>(-halfExtent), brightness, brightness, brightness, alpha});
        vertices.insert(vertices.end(), {fi, 0.0f, static_cast<float>(halfExtent),  brightness, brightness, brightness, alpha});

        // Line along X (Z = i)
        vertices.insert(vertices.end(), {static_cast<float>(-halfExtent), 0.0f, fi, brightness, brightness, brightness, alpha});
        vertices.insert(vertices.end(), {static_cast<float>(halfExtent),  0.0f, fi, brightness, brightness, brightness, alpha});
    }

    // X axis (red)
    vertices.insert(vertices.end(), {static_cast<float>(-halfExtent), 0.0f, 0.0f, 1.0f, 0.2f, 0.2f, axisAlpha});
    vertices.insert(vertices.end(), {static_cast<float>(halfExtent),  0.0f, 0.0f, 1.0f, 0.2f, 0.2f, axisAlpha});

    // Z axis (blue)
    vertices.insert(vertices.end(), {0.0f, 0.0f, static_cast<float>(-halfExtent), 0.2f, 0.2f, 1.0f, axisAlpha});
    vertices.insert(vertices.end(), {0.0f, 0.0f, static_cast<float>(halfExtent),  0.2f, 0.2f, 1.0f, axisAlpha});

    // Y axis (green) -- vertical line at the origin
    vertices.insert(vertices.end(), {0.0f, static_cast<float>(-halfExtent), 0.0f, 0.2f, 1.0f, 0.2f, axisAlpha});
    vertices.insert(vertices.end(), {0.0f, static_cast<float>(halfExtent),  0.0f, 0.2f, 1.0f, 0.2f, axisAlpha});

    m_gridVertexCount = static_cast<int>(vertices.size()) / 7; // 7 floats per vertex

    glGenVertexArrays(1, &m_gridVAO);
    glGenBuffers(1, &m_gridVBO);

    glBindVertexArray(m_gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_gridVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
                 vertices.data(), GL_STATIC_DRAW);

    // Position attribute (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), reinterpret_cast<void*>(0));

    // Color attribute (location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Renderer::initGizmo() {
    // Gizmo: 3 axis lines from origin, each 1 unit long
    // Each vertex: position(3) + color(3)
    float gizmoVerts[] = {
        // X axis (red)
        0.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
        // Y axis (green)
        0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,   0.0f, 1.0f, 0.0f,
        // Z axis (blue)
        0.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f,

        // X axis arrowhead
        1.0f, 0.0f, 0.0f,    1.0f, 0.0f, 0.0f,
        0.85f, 0.05f, 0.0f,  1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,    1.0f, 0.0f, 0.0f,
        0.85f, -0.05f, 0.0f, 1.0f, 0.0f, 0.0f,

        // Y axis arrowhead
        0.0f, 1.0f, 0.0f,    0.0f, 1.0f, 0.0f,
        0.05f, 0.85f, 0.0f,  0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,    0.0f, 1.0f, 0.0f,
        -0.05f, 0.85f, 0.0f, 0.0f, 1.0f, 0.0f,

        // Z axis arrowhead
        0.0f, 0.0f, 1.0f,    0.0f, 0.0f, 1.0f,
        0.0f, 0.05f, 0.85f,  0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,    0.0f, 0.0f, 1.0f,
        0.0f, -0.05f, 0.85f, 0.0f, 0.0f, 1.0f,
    };

    glGenVertexArrays(1, &m_gizmoVAO);
    glGenBuffers(1, &m_gizmoVBO);

    glBindVertexArray(m_gizmoVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_gizmoVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gizmoVerts), gizmoVerts, GL_STATIC_DRAW);

    // Position (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(0));

    // Color (location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Renderer::beginFrame(const glm::mat4& view, const glm::mat4& projection) {
    m_viewMatrix = view;
    m_projMatrix = projection;
}

void Renderer::endFrame() {
    // Reserved for future post-processing or swap logic
}

void Renderer::clear(float r, float g, float b, float a) {
    if (!m_initialized) return;
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::resize(int width, int height) {
    if (!m_initialized) return;
    glViewport(0, 0, width, height);
}

void Renderer::drawMesh(MeshBuffer& meshBuffer, const glm::mat4& modelMatrix, const Material& material) {
    if (!m_initialized) return;

    auto shader = m_shaderLibrary.get("blinn_phong");
    if (!shader) return;

    shader->bind();
    shader->setMat4("uModel", modelMatrix);
    shader->setMat4("uView", m_viewMatrix);
    shader->setMat4("uProjection", m_projMatrix);

    // Normal matrix for correct lighting with non-uniform scaling
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
    shader->setMat3("uNormalMatrix", normalMatrix);

    // Material uniforms
    shader->setVec3("uAlbedo", material.albedo);
    shader->setFloat("uMetallic", material.metallic);
    shader->setFloat("uRoughness", material.roughness);

    // Camera position for specular
    glm::mat4 invView = glm::inverse(m_viewMatrix);
    glm::vec3 cameraPos = glm::vec3(invView[3]);
    shader->setVec3("uCameraPos", cameraPos);

    // Albedo texture
    if (material.albedoMap) {
        material.albedoMap->bind(0);
        shader->setInt("uAlbedoMap", 0);
        shader->setInt("uHasAlbedoMap", 1);
    } else {
        shader->setInt("uHasAlbedoMap", 0);
    }

    // Lights
    int numLights = static_cast<int>(m_lights.size());
    if (numLights > 8) numLights = 8;
    shader->setInt("uNumLights", numLights);

    for (int i = 0; i < numLights; ++i) {
        std::string prefix = "uLights[" + std::to_string(i) + "].";
        shader->setInt(prefix + "type", static_cast<int>(m_lights[static_cast<size_t>(i)].type));
        shader->setVec3(prefix + "position", m_lights[static_cast<size_t>(i)].position);
        shader->setVec3(prefix + "direction", m_lights[static_cast<size_t>(i)].direction);
        shader->setVec3(prefix + "color", m_lights[static_cast<size_t>(i)].color);
        shader->setFloat(prefix + "intensity", m_lights[static_cast<size_t>(i)].intensity);
        shader->setFloat(prefix + "constant", m_lights[static_cast<size_t>(i)].constant);
        shader->setFloat(prefix + "linear", m_lights[static_cast<size_t>(i)].linear);
        shader->setFloat(prefix + "quadratic", m_lights[static_cast<size_t>(i)].quadratic);
    }

    // Draw the mesh (MeshBuffer handles VAO binding internally)
    meshBuffer.draw();

    shader->unbind();

    // Wireframe overlay
    if (m_wireframeEnabled) {
        drawWireframe(meshBuffer, modelMatrix);
    }
}

void Renderer::drawWireframe(MeshBuffer& meshBuffer, const glm::mat4& modelMatrix, const glm::vec3& color) {
    if (!m_initialized) return;

    auto shader = m_shaderLibrary.get("wireframe");
    if (!shader) return;

    shader->bind();
    shader->setMat4("uModel", modelMatrix);
    shader->setMat4("uView", m_viewMatrix);
    shader->setMat4("uProjection", m_projMatrix);
    shader->setVec3("uColor", color);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_POLYGON_OFFSET_LINE);
    glPolygonOffset(-1.0f, -1.0f);

    // Draw wireframe using the MeshBuffer's wireframe data
    meshBuffer.drawWireframe();

    glDisable(GL_POLYGON_OFFSET_LINE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    shader->unbind();
}

void Renderer::drawGrid() {
    if (!m_initialized || !m_gridEnabled) return;

    auto shader = m_shaderLibrary.get("grid");
    if (!shader) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    shader->bind();
    shader->setMat4("uView", m_viewMatrix);
    shader->setMat4("uProjection", m_projMatrix);

    glBindVertexArray(m_gridVAO);
    glDrawArrays(GL_LINES, 0, m_gridVertexCount);
    glBindVertexArray(0);

    shader->unbind();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void Renderer::drawGizmo(const glm::vec3& position, int /*mode*/, int highlightAxis) {
    if (!m_initialized) return;

    auto shader = m_shaderLibrary.get("gizmo");
    if (!shader) return;

    // Disable depth test so gizmo is always visible
    glDisable(GL_DEPTH_TEST);
    glLineWidth(2.0f);

    shader->bind();
    shader->setMat4("uView", m_viewMatrix);
    shader->setMat4("uProjection", m_projMatrix);

    // Translate gizmo to the object position
    glm::mat4 model = glm::mat4(1.0f);
    model[3] = glm::vec4(position, 1.0f);
    shader->setMat4("uModel", model);

    glBindVertexArray(m_gizmoVAO);

    // Draw all 3 axis lines (6 vertices) + 3 arrowheads (12 vertices) = 18 vertices total
    // But we can highlight specific axes by drawing them separately if needed
    (void)highlightAxis; // Reserved for future highlight logic
    glDrawArrays(GL_LINES, 0, 18);

    glBindVertexArray(0);
    shader->unbind();

    glLineWidth(1.0f);
    glEnable(GL_DEPTH_TEST);
}

void Renderer::setLights(const std::vector<LightData>& lights) {
    m_lights = lights;
}

} // namespace PixellentModeler
