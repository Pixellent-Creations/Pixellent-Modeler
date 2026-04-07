#include "PixellentModeler/Core/Window.hpp"
#include "PixellentModeler/Core/EventDispatcher.hpp"
#include "PixellentModeler/Core/Log.hpp"
#include <glad/glad.h>

namespace PixellentModeler {

Window::Window(int width, int height, const char* title)
    : m_width(width), m_height(height)
{
    Log::info("Initializing GLFW window");

    if (!glfwInit()) {
        Log::error("Failed to initialize GLFW");
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
        Log::error("Failed to create GLFW window");
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(m_window);

    // Initialize GLAD after the context is current
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        Log::error("Failed to initialize GLAD");
        return;
    }

    // Store this pointer for use in static callbacks
    glfwSetWindowUserPointer(m_window, this);

    setupCallbacks();

    Log::info("Window created successfully (" + std::to_string(width) + "x" + std::to_string(height) + ")");
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
    if (m_window) {
        glfwPollEvents();
    }
}

void Window::swapBuffers() {
    if (m_window) {
        glfwSwapBuffers(m_window);
    }
}

int Window::framebufferWidth() const {
    int w = 0, h = 0;
    if (m_window) {
        glfwGetFramebufferSize(m_window, &w, &h);
    }
    return w;
}

int Window::framebufferHeight() const {
    int w = 0, h = 0;
    if (m_window) {
        glfwGetFramebufferSize(m_window, &w, &h);
    }
    return h;
}

void Window::setupCallbacks() {
    glfwSetKeyCallback(m_window, keyCallback);
    glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
    glfwSetCursorPosCallback(m_window, cursorPosCallback);
    glfwSetScrollCallback(m_window, scrollCallback);
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
    glfwSetWindowCloseCallback(m_window, windowCloseCallback);
}

void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (!self || !self->m_dispatcher) return;

    EventType type = EventType::None;
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        type = EventType::KeyPressed;
    } else if (action == GLFW_RELEASE) {
        type = EventType::KeyReleased;
    }

    if (type != EventType::None) {
        KeyEvent event(type, key, scancode, action, mods);
        self->m_dispatcher->dispatch(event);
    }
}

void Window::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (!self || !self->m_dispatcher) return;

    EventType type = (action == GLFW_PRESS)
        ? EventType::MouseButtonPressed
        : EventType::MouseButtonReleased;

    MouseButtonEvent event(type, button, action, mods);
    self->m_dispatcher->dispatch(event);
}

void Window::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (!self || !self->m_dispatcher) return;

    MouseMoveEvent event(xpos, ypos);
    self->m_dispatcher->dispatch(event);
}

void Window::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (!self || !self->m_dispatcher) return;

    MouseScrollEvent event(xoffset, yoffset);
    self->m_dispatcher->dispatch(event);
}

void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (!self || !self->m_dispatcher) return;

    self->m_width = width;
    self->m_height = height;

    glViewport(0, 0, width, height);

    WindowResizeEvent event(width, height);
    self->m_dispatcher->dispatch(event);
}

void Window::windowCloseCallback(GLFWwindow* window) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (!self || !self->m_dispatcher) return;

    WindowCloseEvent event;
    self->m_dispatcher->dispatch(event);
}

} // namespace PixellentModeler
