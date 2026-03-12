#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Shader.hpp"

namespace PixellentModeler {

class Renderer {
public:
    // Create the renderer for a given GLFW window. The window must have a
    // valid OpenGL context (made current) before constructing the renderer.
    explicit Renderer(GLFWwindow* window);
    ~Renderer();

    // clear the frame buffer with the given color and depth
    void clear(float r, float g, float b, float a = 1.0f);

    // adjust viewport when the framebuffer resizes
    void resize(int width, int height);

    // draw a simple test triangle using a provided model‑view‑projection matrix
    void drawTriangle(const glm::mat4& mvp);

    // swap the GLFW buffers associated with the window
    void present();

    bool initialized() const { return m_initialized; }

private:
    GLFWwindow* m_window = nullptr;
    bool m_initialized = false;

    // triangle resources
    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    Shader m_shader;
};

} // namespace PixellentModeler
