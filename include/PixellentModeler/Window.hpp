#pragma once

#include <GLFW/glfw3.h>

namespace PixellentModeler {

class Window {
public:
    Window(int width = 800, int height = 600, const char* title = "Pixellent Modeler");
    ~Window();

    bool shouldClose() const;
    void pollEvents();
    void swapBuffers();

    GLFWwindow* native() const { return m_window; }

private:
    GLFWwindow* m_window = nullptr;
};

} // namespace PixellentModeler
