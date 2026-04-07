#include "PixellentModeler/UI/MenuBar.hpp"

#include "imgui.h"

namespace PixellentModeler {

void MenuBar::render(const Actions& actions, CommandHistory* /*history*/, Renderer* /*renderer*/) {
    if (ImGui::BeginMainMenuBar()) {
        // ---- File menu ----
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
                if (actions.onNewScene) actions.onNewScene();
            }
            if (ImGui::MenuItem("Open Project...", "Ctrl+O")) {
                if (actions.onOpenProject) actions.onOpenProject();
            }
            if (ImGui::MenuItem("Save Project", "Ctrl+S")) {
                if (actions.onSaveProject) actions.onSaveProject();
            }
            if (ImGui::MenuItem("Save Project As...", "Ctrl+Shift+S")) {
                if (actions.onSaveProjectAs) actions.onSaveProjectAs();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Import OBJ...")) {
                if (actions.onImportObj) actions.onImportObj();
            }
            if (ImGui::MenuItem("Export OBJ...")) {
                if (actions.onExportObj) actions.onExportObj();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Exit")) {
                if (actions.onExit) actions.onExit();
            }

            ImGui::EndMenu();
        }

        // ---- Edit menu ----
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {
                if (actions.onUndo) actions.onUndo();
            }
            if (ImGui::MenuItem("Redo", "Ctrl+Shift+Z")) {
                if (actions.onRedo) actions.onRedo();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Delete", "X")) {
                if (actions.onDelete) actions.onDelete();
            }
            if (ImGui::MenuItem("Duplicate", "Ctrl+D")) {
                if (actions.onDuplicate) actions.onDuplicate();
            }
            if (ImGui::MenuItem("Select All", "A")) {
                if (actions.onSelectAll) actions.onSelectAll();
            }

            ImGui::EndMenu();
        }

        // ---- View menu ----
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Toggle Wireframe")) {
                if (actions.onToggleWireframe) actions.onToggleWireframe();
            }
            if (ImGui::MenuItem("Toggle Grid")) {
                if (actions.onToggleGrid) actions.onToggleGrid();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Reset Camera")) {
                if (actions.onResetCamera) actions.onResetCamera();
            }

            ImGui::EndMenu();
        }

        // ---- Add menu ----
        if (ImGui::BeginMenu("Add")) {
            // Mesh primitives
            const char* primitives[] = {
                "Cube", "Sphere", "Cylinder", "Cone", "Plane", "Torus"
            };
            for (const char* prim : primitives) {
                if (ImGui::MenuItem(prim)) {
                    if (actions.onAddPrimitive) actions.onAddPrimitive(prim);
                }
            }

            ImGui::Separator();

            // Lights
            if (ImGui::MenuItem("Point Light")) {
                if (actions.onAddPointLight) actions.onAddPointLight();
            }
            if (ImGui::MenuItem("Directional Light")) {
                if (actions.onAddDirectionalLight) actions.onAddDirectionalLight();
            }

            ImGui::EndMenu();
        }

        // ---- Help menu ----
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                m_showAbout = true;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    // ---- About popup ----
    if (m_showAbout) {
        ImGui::OpenPopup("About Pixellent Modeler");
        m_showAbout = false;
    }

    if (ImGui::BeginPopupModal("About Pixellent Modeler", nullptr,
                                ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Pixellent Modeler");
        ImGui::Separator();
        ImGui::Text("Version 0.1.0");
        ImGui::Text("A lightweight 3D modeling application");
        ImGui::Text("Built with OpenGL 3.3, GLFW, and Dear ImGui");
        ImGui::Spacing();

        float buttonWidth = 120.0f;
        float windowWidth = ImGui::GetWindowSize().x;
        ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
        if (ImGui::Button("Close", ImVec2(buttonWidth, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

} // namespace PixellentModeler
