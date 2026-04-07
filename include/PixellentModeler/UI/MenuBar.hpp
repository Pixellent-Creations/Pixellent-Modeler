#pragma once

#include <functional>
#include <string>

namespace PixellentModeler {

class CommandHistory;
class Renderer;

class MenuBar {
public:
    struct Actions {
        std::function<void()> onNewScene;
        std::function<void()> onOpenProject;
        std::function<void()> onSaveProject;
        std::function<void()> onSaveProjectAs;
        std::function<void()> onImportObj;
        std::function<void()> onExportObj;
        std::function<void()> onExit;
        std::function<void()> onUndo;
        std::function<void()> onRedo;
        std::function<void()> onDelete;
        std::function<void()> onDuplicate;
        std::function<void()> onSelectAll;
        std::function<void(const std::string&)> onAddPrimitive; // "Cube", "Sphere", etc.
        std::function<void()> onAddPointLight;
        std::function<void()> onAddDirectionalLight;
        std::function<void()> onToggleWireframe;
        std::function<void()> onToggleGrid;
        std::function<void()> onResetCamera;
    };

    void render(const Actions& actions, CommandHistory* history, Renderer* renderer);

private:
    bool m_showAbout = false;
};

} // namespace PixellentModeler
