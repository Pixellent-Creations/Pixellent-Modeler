#pragma once

#include <cstdint>
#include <vector>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#include "PixellentModeler/Rendering/ShaderLibrary.hpp"
#include "PixellentModeler/Rendering/Light.hpp"
#include "PixellentModeler/Rendering/Material.hpp"

namespace PixellentModeler {

class MeshBuffer;

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool init(GLFWwindow* window);

    void beginFrame(const glm::mat4& view, const glm::mat4& projection);
    void endFrame();

    void clear(float r, float g, float b, float a = 1.0f);
    void resize(int width, int height);

    void drawMesh(MeshBuffer& meshBuffer, const glm::mat4& modelMatrix, const Material& material);
    void drawWireframe(MeshBuffer& meshBuffer, const glm::mat4& modelMatrix, const glm::vec3& color = glm::vec3(0.0f));
    void drawGrid();
    void drawGizmo(const glm::vec3& position, int mode, int highlightAxis);

    void setLights(const std::vector<LightData>& lights);
    void setWireframeEnabled(bool enabled) { m_wireframeEnabled = enabled; }
    void setGridEnabled(bool enabled) { m_gridEnabled = enabled; }
    bool wireframeEnabled() const { return m_wireframeEnabled; }
    bool gridEnabled() const { return m_gridEnabled; }

    ShaderLibrary& shaderLibrary() { return m_shaderLibrary; }

    bool initialized() const { return m_initialized; }

private:
    bool m_initialized = false;
    GLFWwindow* m_window = nullptr;

    ShaderLibrary m_shaderLibrary;
    std::vector<LightData> m_lights;

    glm::mat4 m_viewMatrix{1.0f};
    glm::mat4 m_projMatrix{1.0f};

    bool m_wireframeEnabled = true;
    bool m_gridEnabled = true;

    // Grid resources
    uint32_t m_gridVAO = 0;
    uint32_t m_gridVBO = 0;
    int m_gridVertexCount = 0;

    // Gizmo resources
    uint32_t m_gizmoVAO = 0;
    uint32_t m_gizmoVBO = 0;

    void initGrid();
    void initGizmo();
    void initShaders();
};

} // namespace PixellentModeler
