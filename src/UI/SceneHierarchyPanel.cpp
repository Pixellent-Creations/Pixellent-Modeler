#include "PixellentModeler/UI/SceneHierarchyPanel.hpp"
#include "PixellentModeler/Scene/Scene.hpp"
#include "PixellentModeler/Scene/Entity.hpp"
#include "PixellentModeler/Selection/SelectionManager.hpp"

#include "imgui.h"

#include <string>

namespace PixellentModeler {

void SceneHierarchyPanel::render(Scene& scene, SelectionManager* selection) {
    ImGui::Begin("Scene Hierarchy");

    // Iterate through all entities in the scene and display root nodes
    const auto& entities = scene.entities();
    for (const auto& entity : entities) {
        // Only draw root-level entities (no parent)
        if (entity->parent() == nullptr) {
            drawEntityNode(*entity, scene, selection);
        }
    }

    // Right-click on blank space to add new entity
    if (ImGui::BeginPopupContextWindow("HierarchyContextMenu",
                                        ImGuiPopupFlags_NoOpenOverItems |
                                        ImGuiPopupFlags_MouseButtonRight))
    {
        if (ImGui::MenuItem("Add Empty Entity")) {
            scene.createEntity("Empty Entity");
        }
        ImGui::EndPopup();
    }

    // Deselect when clicking on empty space
    if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
        m_selectedEntity = INVALID_ENTITY;
        if (selection) {
            selection->clearSelection();
        }
    }

    ImGui::End();
}

void SceneHierarchyPanel::drawEntityNode(Entity& entity, Scene& scene, SelectionManager* selection) {
    EntityID id = entity.id();
    const std::string& name = entity.name();

    // Determine tree node flags
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
                             | ImGuiTreeNodeFlags_OpenOnDoubleClick
                             | ImGuiTreeNodeFlags_SpanAvailWidth;

    bool isSelected = (m_selectedEntity == id);
    if (isSelected) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    // Leaf nodes (no children) get a bullet instead of an arrow
    if (entity.children().empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }

    bool nodeOpen = ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<uintptr_t>(id)),
                                       flags, "%s", name.c_str());

    // Selection handling
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        m_selectedEntity = id;
        if (selection) {
            selection->select(id);
        }
    }

    // Right-click context menu on this node
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Rename")) {
            // Rename action would be handled by a modal or inline edit
        }
        if (ImGui::MenuItem("Duplicate")) {
            scene.duplicateEntity(id);
        }
        if (ImGui::MenuItem("Delete")) {
            scene.destroyEntity(id);
            if (m_selectedEntity == id) {
                m_selectedEntity = INVALID_ENTITY;
                if (selection) {
                    selection->clearSelection();
                }
            }
        }
        ImGui::EndPopup();
    }

    // Draw children recursively
    if (nodeOpen && !entity.children().empty()) {
        for (Entity* child : entity.children()) {
            drawEntityNode(*child, scene, selection);
        }
        ImGui::TreePop();
    }
}

} // namespace PixellentModeler
