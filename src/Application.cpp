#include "PixellentModeler/Application.hpp"
#include "PixellentModeler/Core.hpp"
#include "PixellentModeler/Window.hpp"
#include "PixellentModeler/Renderer.hpp"
#include "PixellentModeler/Scene.hpp"
#include <glm/glm.hpp>
#include <iostream>

namespace PixellentModeler {

class Application {
public:
    Application() {
        std::cout << "Application created" << std::endl;
        m_scene = std::make_unique<Scene>();
        m_window = std::make_unique<Window>(800, 600, "Pixellent Modeler");
        if (m_window && m_window->native()) {
            m_renderer = std::make_unique<Renderer>(m_window->native());
            if (!m_renderer->initialized()) {
                std::cerr << "Renderer failed to initialize" << std::endl;
            }
        }
        if (m_scene && m_window) {
            // make sure camera aspect matches initial window size
            int w, h;
            glfwGetFramebufferSize(m_window->native(), &w, &h);
            m_scene->camera().setAspectRatio(w > 0 && h > 0 ? (float)w / h : 4.0f/3.0f);
        }
    }
    void run() {
        std::cout << "Running main loop" << std::endl;
        while (m_window && !m_window->shouldClose()) {
            m_window->pollEvents();
            // update camera aspect ratio/renderer viewport if window size changed
            if (m_window && m_scene) {
                int w, h;
                glfwGetFramebufferSize(m_window->native(), &w, &h);
                if (h > 0) {
                    float aspect = (float)w / h;
                    m_scene->camera().setAspectRatio(aspect);
                    if (m_renderer)
                        m_renderer->resize(w, h);
                }
            }
            if (m_renderer && m_renderer->initialized()) {
                m_renderer->clear(0.1f, 0.1f, 0.1f, 1.0f);
                if (m_scene) {
                    glm::mat4 mvp = m_scene->camera().projection() * m_scene->camera().view();
                    m_renderer->drawTriangle(mvp);
                }
            }
            if (m_renderer)
                m_renderer->present();
        }
    }

private:
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
};

} // namespace PixellentModeler
