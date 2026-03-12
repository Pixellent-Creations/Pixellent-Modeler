#include "PixellentModeler/Application.hpp"
#include "PixellentModeler/Core.hpp"
#include "PixellentModeler/Window.hpp"
#include "PixellentModeler/Renderer.hpp"
#include <iostream>

namespace PixellentModeler {

class Application {
public:
    Application() {
        std::cout << "Application created" << std::endl;
        m_window = std::make_unique<Window>(800, 600, "Pixellent Modeler");
        if (m_window && m_window->native()) {
            m_renderer = std::make_unique<Renderer>(m_window->native());
            if (!m_renderer->initialized()) {
                std::cerr << "Renderer failed to initialize" << std::endl;
            }
        }
    }
    void run() {
        std::cout << "Running main loop" << std::endl;
        while (m_window && !m_window->shouldClose()) {
            m_window->pollEvents();
            if (m_renderer && m_renderer->initialized()) {
                m_renderer->clear(0.1f, 0.1f, 0.1f, 1.0f);
            }
            // TODO: update/render scene
            if (m_renderer)
                m_renderer->present();
        }
    }

private:
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
};

} // namespace PixellentModeler
