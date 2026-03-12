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

    // set default viewport size based on window
    int w, h;
    glfwGetFramebufferSize(m_window, &w, &h);
    glViewport(0, 0, w, h);

    // prepare a simple triangle and shader
    float vertices[] = {
        // positions         // colors
         0.0f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,
         0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f
    };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);

    const std::string vertSrc =
        "#version 330 core\n"
        "layout(location = 0) in vec3 aPos;\n"
        "layout(location = 1) in vec3 aColor;\n"
        "uniform mat4 uMVP;\n"
        "out vec3 vColor;\n"
        "void main() { vColor = aColor; gl_Position = uMVP * vec4(aPos, 1.0); }\n";

    const std::string fragSrc =
        "#version 330 core\n"
        "in vec3 vColor;\n"
        "out vec4 FragColor;\n"
        "void main() { FragColor = vec4(vColor, 1.0); }\n";

    if (!m_shader.create(vertSrc, fragSrc)) {
        std::cerr << "Failed to create triangle shader" << std::endl;
    }
}

Renderer::~Renderer() {
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
}

void Renderer::clear(float r, float g, float b, float a) {
    if (!m_initialized)
        return;
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::resize(int width, int height) {
    if (!m_initialized)
        return;
    glViewport(0, 0, width, height);
}

void Renderer::drawTriangle(const glm::mat4& mvp) {
    if (!m_initialized)
        return;
    m_shader.bind();
    m_shader.setMat4("uMVP", mvp);
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    m_shader.unbind();
}

void Renderer::present() {
    if (m_window)
        glfwSwapBuffers(m_window);
}

} // namespace PixellentModeler
