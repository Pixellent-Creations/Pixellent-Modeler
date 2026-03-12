#pragma once

#include <GLFW/glfw3.h>

namespace PixellentModeler {

class Renderer {
public:
    // Create the renderer for a given GLFW window. The window must have a
    // valid OpenGL context (made current) before constructing the renderer.
    explicit Renderer(GLFWwindow* window);
    ~Renderer();

    // clear the frame buffer with the given color and depth
    void clear(float r, float g, float b, float a = 1.0f);

    // swap the GLFW buffers associated with the window
    void present();

    bool initialized() const { return m_initialized; }

private:
    GLFWwindow* m_window = nullptr;
    bool m_initialized = false;
};

} // namespace PixellentModeler
