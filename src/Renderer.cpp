#include "PixellentModeler/Renderer.hpp"
#include <glad/glad.h>
#include <iostream>

namespace PixellentModeler {

Renderer::Renderer(GLFWwindow* window)
    : m_window(window)
{
    if (!m_window) {
        std::cerr << "Renderer requires a valid GLFWwindow pointer" << std::endl;
        return;
    }

    // context should already be current on this thread
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD in Renderer" << std::endl;
        return;
    }

    m_initialized = true;

    // basic OpenGL state
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glViewport(0, 0, 800, 600); // default; user can adjust later
}

Renderer::~Renderer() {
    // nothing to do; GLFW will destroy context when window is destroyed
}

void Renderer::clear(float r, float g, float b, float a) {
    if (!m_initialized)
        return;
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::present() {
    if (m_window)
        glfwSwapBuffers(m_window);
}

} // namespace PixellentModeler
