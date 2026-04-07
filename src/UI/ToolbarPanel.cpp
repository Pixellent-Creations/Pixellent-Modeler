#include "PixellentModeler/UI/ToolbarPanel.hpp"
#include "PixellentModeler/Tools/ToolManager.hpp"

#include "imgui.h"

namespace PixellentModeler {

void ToolbarPanel::render(ToolManager* toolManager) {
    ImGui::Begin("Toolbar");

    std::string activeTool = toolManager ? toolManager->activeToolName() : "";

    // Selection tool
    {
        bool active = (activeTool == "Select");
        toolButton("Select", "Select  (Q)", active);
        if (ImGui::IsItemClicked() && toolManager) {
            toolManager->setActiveTool("Select");
        }
    }

    // Move tool
    {
        bool active = (activeTool == "Move");
        toolButton("Move", "Move  (W)", active);
        if (ImGui::IsItemClicked() && toolManager) {
            toolManager->setActiveTool("Move");
        }
    }

    // Rotate tool
    {
        bool active = (activeTool == "Rotate");
        toolButton("Rotate", "Rotate  (E)", active);
        if (ImGui::IsItemClicked() && toolManager) {
            toolManager->setActiveTool("Rotate");
        }
    }

    // Scale tool
    {
        bool active = (activeTool == "Scale");
        toolButton("Scale", "Scale  (R)", active);
        if (ImGui::IsItemClicked() && toolManager) {
            toolManager->setActiveTool("Scale");
        }
    }

    ImGui::Separator();

    // Extrude tool
    {
        bool active = (activeTool == "Extrude");
        toolButton("Extrude", "Extrude  (Shift+E)", active);
        if (ImGui::IsItemClicked() && toolManager) {
            toolManager->setActiveTool("Extrude");
        }
    }

    ImGui::End();
}

void ToolbarPanel::toolButton(const char* label, const char* tooltip, bool active) {
    if (active) {
        // Highlight the active tool with the accent color
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.30f, 0.55f, 0.85f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(0.36f, 0.62f, 0.90f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(0.42f, 0.68f, 0.95f, 1.00f));
    }

    float buttonWidth = ImGui::GetContentRegionAvail().x;
    ImGui::Button(label, ImVec2(buttonWidth, 0));

    if (active) {
        ImGui::PopStyleColor(3);
    }

    // Show tooltip with keyboard shortcut on hover
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
        ImGui::SetTooltip("%s", tooltip);
    }
}

} // namespace PixellentModeler
