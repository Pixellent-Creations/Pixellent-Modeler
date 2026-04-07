#pragma once

#include <GLFW/glfw3.h>

namespace PixellentModeler {

class EventDispatcher;

class Window {
public:
    Window(int width = 800, int height = 600, const char* title = "Pixellent Modeler");
    ~Window();

    bool shouldClose() const;
    void pollEvents();
    void swapBuffers();

    int width() const { return m_width; }
    int height() const { return m_height; }
    int framebufferWidth() const;
    int framebufferHeight() const;

    void setEventDispatcher(EventDispatcher* dispatcher) { m_dispatcher = dispatcher; }

    GLFWwindow* native() const { return m_window; }

private:
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void windowCloseCallback(GLFWwindow* window);

    void setupCallbacks();

    GLFWwindow* m_window = nullptr;
    EventDispatcher* m_dispatcher = nullptr;
    int m_width = 800;
    int m_height = 600;
};

} // namespace PixellentModeler
