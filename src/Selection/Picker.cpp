#include "PixellentModeler/Selection/Picker.hpp"
#include "PixellentModeler/Rendering/Framebuffer.hpp"
#include "PixellentModeler/Rendering/Shader.hpp"
#include <glad/glad.h>
#include <string>

namespace PixellentModeler {

// ---------------------------------------------------------------------------
// Pick shader source
// ---------------------------------------------------------------------------

static const std::string kPickVertexShader = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 uMVP;
void main() {
    gl_Position = uMVP * vec4(aPos, 1.0);
}
)";

static const std::string kPickFragmentShader = R"(
#version 330 core
uniform int uEntityID;
out vec4 FragColor;
void main() {
    int id = uEntityID + 1; // offset by 1 so 0 means "nothing"
    float r = float(id & 0xFF) / 255.0;
    float g = float((id >> 8) & 0xFF) / 255.0;
    float b = float((id >> 16) & 0xFF) / 255.0;
    FragColor = vec4(r, g, b, 1.0);
}
)";

// ---------------------------------------------------------------------------
// Picker implementation
// ---------------------------------------------------------------------------

void Picker::init(int width, int height) {
    m_width = width;
    m_height = height;

    // Create the off-screen framebuffer with 1 RGBA8 color attachment + depth
    FramebufferSpec spec;
    spec.width = width;
    spec.height = height;
    spec.depthAttachment = true;
    spec.colorAttachments = 1;
    m_framebuffer = std::make_unique<Framebuffer>(spec);

    // Create the pick shader
    m_pickShader = std::make_unique<Shader>();
    m_pickShader->create(kPickVertexShader, kPickFragmentShader);
}

void Picker::resize(int width, int height) {
    m_width = width;
    m_height = height;
    if (m_framebuffer) {
        m_framebuffer->resize(width, height);
    }
}

void Picker::beginPick() {
    if (!m_framebuffer) return;

    m_framebuffer->bind();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
}

void Picker::renderEntityForPick(MeshBuffer& /*meshBuffer*/, const glm::mat4& mvp,
                                  EntityID entityId) {
    if (!m_pickShader) return;

    m_pickShader->bind();
    m_pickShader->setMat4("uMVP", mvp);

    // Set the entity ID uniform directly via GL since Shader only has setMat4
    int loc = glGetUniformLocation(
        // We need the program handle; use the bound program
        0, "uEntityID");
    // Use the currently bound program to find the uniform
    GLint currentProgram = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    loc = glGetUniformLocation(static_cast<GLuint>(currentProgram), "uEntityID");
    glUniform1i(loc, static_cast<GLint>(entityId));

    // Draw the mesh buffer
    // TODO: Call meshBuffer.bind() and meshBuffer.draw() once MeshBuffer
    // class is fully implemented. For now this is a placeholder.
    // meshBuffer.bind();
    // meshBuffer.draw();

    m_pickShader->unbind();
}

void Picker::endPick() {
    if (m_framebuffer) {
        m_framebuffer->unbind();
    }
}

EntityID Picker::pickEntity(int mouseX, int mouseY) const {
    if (!m_framebuffer) return INVALID_ENTITY;

    // OpenGL reads from bottom-left, so flip Y
    int readX = mouseX;
    int readY = m_height - mouseY - 1;

    // Read the RGBA pixel at the given position
    uint8_t pixel[4] = {0, 0, 0, 0};
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebuffer->fbo());
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(readX, readY, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    // If alpha is 0, nothing was rendered at this pixel
    if (pixel[3] == 0) {
        return INVALID_ENTITY;
    }

    // Decode entity ID from RGB (reverse the +1 offset applied in the shader)
    uint32_t id = static_cast<uint32_t>(pixel[0])
                | (static_cast<uint32_t>(pixel[1]) << 8)
                | (static_cast<uint32_t>(pixel[2]) << 16);
    id -= 1; // undo the +1 offset

    return static_cast<EntityID>(id);
}

} // namespace PixellentModeler
