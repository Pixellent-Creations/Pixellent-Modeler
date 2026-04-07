#include "PixellentModeler/UI/UIManager.hpp"
#include "PixellentModeler/Core/Application.hpp"
#include "PixellentModeler/Scene/Scene.hpp"
#include "PixellentModeler/Rendering/Renderer.hpp"
#include "PixellentModeler/Core/Window.hpp"

#include "imgui.h"
#include "imgui_internal.h" // Required for DockBuilder functions

#include <iostream>

namespace PixellentModeler {

UIManager::UIManager() = default;

UIManager::~UIManager() {
    shutdown();
}

void UIManager::init(GLFWwindow* window) {
    m_imguiLayer.init(window);
    m_viewport.init();
    std::cout << "UIManager initialized" << std::endl;
}

void UIManager::shutdown() {
    m_imguiLayer.shutdown();
}

void UIManager::render(Application& app) {
    if (!m_imguiLayer.initialized()) {
        return;
    }

    // ---- Begin ImGui frame ----
    m_imguiLayer.beginFrame();

    // ---- Full-screen dockspace ----
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

    // ---- Set up default docking layout on first frame ----
    if (m_firstFrame) {
        setupDockingLayout();
        m_firstFrame = false;
    }

    // ---- Wire up MenuBar actions to Application ----
    MenuBar::Actions menuActions;

    menuActions.onNewScene = [&app]() {
        app.newScene();
    };
    menuActions.onOpenProject = [&app]() {
        app.openProject();
    };
    menuActions.onSaveProject = [&app]() {
        app.saveProject();
    };
    menuActions.onSaveProjectAs = [&app]() {
        app.saveProjectAs();
    };
    menuActions.onImportObj = [&app]() {
        app.importObj();
    };
    menuActions.onExportObj = [&app]() {
        app.exportObj();
    };
    menuActions.onExit = [&app]() {
        app.requestExit();
    };
    menuActions.onUndo = [&app]() {
        app.undo();
    };
    menuActions.onRedo = [&app]() {
        app.redo();
    };
    menuActions.onDelete = [&app]() {
        app.deleteSelected();
    };
    menuActions.onDuplicate = [&app]() {
        app.duplicateSelected();
    };
    menuActions.onSelectAll = [&app]() {
        app.selectAll();
    };
    menuActions.onAddPrimitive = [&app](const std::string& name) {
        app.addPrimitive(name);
    };
    menuActions.onAddPointLight = [&app]() {
        app.addPointLight();
    };
    menuActions.onAddDirectionalLight = [&app]() {
        app.addDirectionalLight();
    };
    menuActions.onToggleWireframe = [&app]() {
        app.toggleWireframe();
    };
    menuActions.onToggleGrid = [&app]() {
        app.toggleGrid();
    };
    menuActions.onResetCamera = [&app]() {
        app.resetCamera();
    };

    // ---- Render all panels ----
    m_menuBar.render(menuActions, &app.commandHistory(), app.renderer());

    if (app.renderer() && app.scene()) {
        m_viewport.render(*app.renderer(), *app.scene());
    }

    if (app.scene()) {
        m_hierarchy.render(*app.scene(), app.selectionManager());
        m_properties.render(*app.scene(), app.selectionManager());
    }

    m_toolbar.render(app.toolManager());

    m_statusBar.render(app.scene(), app.selectionManager());

    // ---- End ImGui frame ----
    m_imguiLayer.endFrame();
}

void UIManager::setupDockingLayout() {
    ImGuiID dockspaceID = ImGui::GetID("DockSpace");

    // Only build the layout if it hasn't been persisted yet
    if (ImGui::DockBuilderGetNode(dockspaceID) != nullptr) {
        return; // Layout already exists, don't override user customizations
    }

    // Clear out any existing layout and create a fresh dockspace node
    ImGui::DockBuilderRemoveNode(dockspaceID);
    ImGui::DockBuilderAddNode(dockspaceID,
                               ImGuiDockNodeFlags_DockSpace);

    // Make it fill the viewport
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::DockBuilderSetNodeSize(dockspaceID, viewport->WorkSize);
    ImGui::DockBuilderSetNodePos(dockspaceID, viewport->WorkPos);

    // Split the dockspace:
    //  +---------------------+-------------------+-------------------+
    //  |                     |                   |                   |
    //  |   Scene Hierarchy   |     Viewport      |   Properties     |
    //  |        (20%)        |     (center)      |      (20%)       |
    //  |                     |                   |                   |
    //  |---------------------+                   |                   |
    //  |      Toolbar        |                   |                   |
    //  +---------------------+-------------------+-------------------+

    ImGuiID dockLeft, dockCenter;
    ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Left, 0.20f,
                                 &dockLeft, &dockCenter);

    ImGuiID dockRight;
    ImGui::DockBuilderSplitNode(dockCenter, ImGuiDir_Right, 0.25f,
                                 &dockRight, &dockCenter);

    // Split the left panel: top = hierarchy, bottom = toolbar
    ImGuiID dockLeftTop, dockLeftBottom;
    ImGui::DockBuilderSplitNode(dockLeft, ImGuiDir_Down, 0.35f,
                                 &dockLeftBottom, &dockLeftTop);

    // Assign windows to dock nodes
    ImGui::DockBuilderDockWindow("Scene Hierarchy", dockLeftTop);
    ImGui::DockBuilderDockWindow("Toolbar",         dockLeftBottom);
    ImGui::DockBuilderDockWindow("Viewport",        dockCenter);
    ImGui::DockBuilderDockWindow("Properties",      dockRight);

    ImGui::DockBuilderFinish(dockspaceID);

    std::cout << "Default docking layout applied" << std::endl;
}

} // namespace PixellentModeler
