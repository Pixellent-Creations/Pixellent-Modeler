#include "PixellentModeler/Window.hpp"
#include <glad/glad.h>
#include <iostream>

namespace PixellentModeler {

Window::Window(int width, int height, const char* title) {
    std::cout << "Initializing GLFW window" << std::endl;
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(m_window);

    // initialize GLAD after the context is current
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
    }
}

Window::~Window() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }
}

bool Window::shouldClose() const {
    return m_window && glfwWindowShouldClose(m_window);
}

void Window::pollEvents() {
    if (m_window) glfwPollEvents();
}

void Window::swapBuffers() {
    if (m_window) glfwSwapBuffers(m_window);
}

} // namespace PixellentModeler
