#pragma once

#include <memory>
#include <string>

namespace PixellentModeler {

class EventDispatcher;
class Window;
class InputManager;
class CommandHistory;
class Scene;
class Renderer;
class ImGuiLayer;
class UIManager;
class SelectionManager;
class ToolManager;

class Application {
public:
    Application();
    ~Application();

    void run();

    static Application& instance() { return *s_instance; }
    EventDispatcher& eventDispatcher() { return *m_eventDispatcher; }
    InputManager& inputManager() { return *m_inputManager; }
    CommandHistory& commandHistory() { return *m_commandHistory; }
    Window& window() { return *m_window; }

    // Subsystem accessors (may return nullptr before init)
    Scene* scene() { return m_scene.get(); }
    Renderer* renderer() { return m_renderer.get(); }
    SelectionManager* selectionManager() { return m_selectionManager.get(); }
    ToolManager* toolManager() { return m_toolManager.get(); }

    // Application actions (invoked by UI menu bar)
    void newScene();
    void openProject();
    void saveProject();
    void saveProjectAs();
    void importObj();
    void exportObj();
    void requestExit();
    void undo();
    void redo();
    void deleteSelected();
    void duplicateSelected();
    void selectAll();
    void addPrimitive(const std::string& name);
    void addPointLight();
    void addDirectionalLight();
    void toggleWireframe();
    void toggleGrid();
    void resetCamera();

private:
    static Application* s_instance;
    std::unique_ptr<EventDispatcher> m_eventDispatcher;
    std::unique_ptr<Window> m_window;
    std::unique_ptr<InputManager> m_inputManager;
    std::unique_ptr<CommandHistory> m_commandHistory;
    std::unique_ptr<Scene> m_scene;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<ImGuiLayer> m_imguiLayer;
    std::unique_ptr<UIManager> m_uiManager;
    std::unique_ptr<SelectionManager> m_selectionManager;
    std::unique_ptr<ToolManager> m_toolManager;
    bool m_running = true;
};

} // namespace PixellentModeler
