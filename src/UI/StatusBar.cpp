#include "PixellentModeler/UI/StatusBar.hpp"
#include "PixellentModeler/Scene/Scene.hpp"
#include "PixellentModeler/Scene/Entity.hpp"
#include "PixellentModeler/Scene/MeshComponent.hpp"
#include "PixellentModeler/Mesh/HalfEdgeMesh.hpp"
#include "PixellentModeler/Selection/SelectionManager.hpp"

#include "imgui.h"

namespace PixellentModeler {

void StatusBar::render(Scene* scene, SelectionManager* selection) {
    ImGuiIO& io = ImGui::GetIO();
    m_frameCount++;
    m_fpsUpdateTimer += io.DeltaTime;

    if (m_fpsUpdateTimer >= 0.5f) {
        m_fps = static_cast<float>(m_frameCount) / m_fpsUpdateTimer;
        m_frameTime = m_fpsUpdateTimer / static_cast<float>(m_frameCount) * 1000.0f;
        m_frameCount = 0;
        m_fpsUpdateTimer = 0.0f;
    }

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove;

    ImGui::Begin("##StatusBar", nullptr, flags);

    if (selection) {
        const char* modeStr = "Object";
        SelectionMode mode = selection->mode();
        switch (mode) {
            case SelectionMode::Object: modeStr = "Object"; break;
            case SelectionMode::Vertex: modeStr = "Vertex"; break;
            case SelectionMode::Edge:   modeStr = "Edge";   break;
            case SelectionMode::Face:   modeStr = "Face";   break;
        }
        ImGui::Text("Mode: %s", modeStr);
        ImGui::SameLine();
        ImGui::Text("|");
        ImGui::SameLine();
    }

    if (selection && selection->hasSelection() && scene) {
        EntityID id = selection->selectedEntity();
        Entity* entity = scene->findEntity(id);
        if (entity) {
            ImGui::Text("Selected: %s", entity->name().c_str());

            if (entity->hasComponent<MeshComponent>()) {
                auto* meshComp = entity->getComponent<MeshComponent>();
                if (meshComp && meshComp->mesh) {
                    ImGui::SameLine();
                    ImGui::Text("|");
                    ImGui::SameLine();
                    ImGui::Text("V: %u  E: %u  F: %u",
                                meshComp->mesh->vertexCount(),
                                meshComp->mesh->edgeCount(),
                                meshComp->mesh->faceCount());
                }
            }
        }
    } else {
        ImGui::Text("No selection");
    }

    float fpsTextWidth = ImGui::CalcTextSize("999.9 FPS | 99.99 ms").x;
    float availWidth = ImGui::GetContentRegionAvail().x;
    if (availWidth > fpsTextWidth) {
        ImGui::SameLine(ImGui::GetCursorPosX() + availWidth - fpsTextWidth);
    }

    ImGui::Text("%.1f FPS | %.2f ms", m_fps, m_frameTime);

    ImGui::End();
}

} // namespace PixellentModeler
