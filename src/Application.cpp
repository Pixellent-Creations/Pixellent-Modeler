#include "PixellentModeler/Core.hpp"
#include "PixellentModeler/Window.hpp"
#include <iostream>

namespace PixellentModeler {

class Application {
public:
    Application() {
        std::cout << "Application created" << std::endl;
        m_window = std::make_unique<Window>(800, 600, "Pixellent Modeler");
    }
    void run() {
        std::cout << "Running main loop" << std::endl;
        while (m_window && !m_window->shouldClose()) {
            m_window->pollEvents();
            // TODO: update/render
            m_window->swapBuffers();
        }
    }

private:
    std::unique_ptr<Window> m_window;
};

} // namespace PixellentModeler
