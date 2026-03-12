#pragma once

#include <memory>

namespace PixellentModeler {

class Window;
class Renderer;
class Scene;

class Application {
public:
    Application();
    ~Application() = default;

    void run();

private:
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Scene> m_scene;
};

} // namespace PixellentModeler
