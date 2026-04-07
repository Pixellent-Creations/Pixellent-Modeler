#include "PixellentModeler/UI/PropertiesPanel.hpp"
#include "PixellentModeler/Scene/Scene.hpp"
#include "PixellentModeler/Scene/Entity.hpp"
#include "PixellentModeler/Scene/TransformComponent.hpp"
#include "PixellentModeler/Scene/MaterialComponent.hpp"
#include "PixellentModeler/Scene/LightComponent.hpp"
#include "PixellentModeler/Selection/SelectionManager.hpp"

#include "imgui.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <cstring>

namespace PixellentModeler {

void PropertiesPanel::render(Scene& scene, SelectionManager* selection) {
    ImGui::Begin("Properties");

    if (!selection || !selection->hasSelection()) {
        ImGui::TextDisabled("No entity selected");
        ImGui::End();
        return;
    }

    EntityID selectedID = selection->selectedEntity();
    Entity* entity = scene.findEntity(selectedID);
    if (!entity) {
        ImGui::TextDisabled("Selected entity not found");
        ImGui::End();
        return;
    }

    // ---- Editable entity name ----
    char nameBuffer[256];
    std::strncpy(nameBuffer, entity->name().c_str(), sizeof(nameBuffer) - 1);
    nameBuffer[sizeof(nameBuffer) - 1] = '\0';
    if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) {
        entity->setName(nameBuffer);
    }

    ImGui::Separator();

    // ---- Transform Component ----
    if (entity->hasComponent<TransformComponent>()) {
        auto* transform = entity->getComponent<TransformComponent>();

        bool open = ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen);
        if (open && transform) {
            drawTransformComponent(*transform);
        }
    }

    // ---- Material Component ----
    if (entity->hasComponent<MaterialComponent>()) {
        auto* material = entity->getComponent<MaterialComponent>();

        bool open = ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen);
        if (open && material) {
            drawMaterialComponent(*material);
        }
    }

    // ---- Light Component ----
    if (entity->hasComponent<LightComponent>()) {
        auto* light = entity->getComponent<LightComponent>();

        bool open = ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen);
        if (open && light) {
            drawLightComponent(*light);
        }
    }

    // ---- Add Component button ----
    ImGui::Separator();
    float buttonWidth = ImGui::GetContentRegionAvail().x;
    if (ImGui::Button("Add Component", ImVec2(buttonWidth, 0))) {
        ImGui::OpenPopup("AddComponentPopup");
    }

    if (ImGui::BeginPopup("AddComponentPopup")) {
        if (!entity->hasComponent<TransformComponent>()) {
            if (ImGui::MenuItem("Transform")) {
                entity->addComponent<TransformComponent>();
            }
        }
        if (!entity->hasComponent<MaterialComponent>()) {
            if (ImGui::MenuItem("Material")) {
                entity->addComponent<MaterialComponent>();
            }
        }
        if (!entity->hasComponent<LightComponent>()) {
            if (ImGui::MenuItem("Light")) {
                entity->addComponent<LightComponent>();
            }
        }
        ImGui::EndPopup();
    }

    ImGui::End();
}

void PropertiesPanel::drawTransformComponent(TransformComponent& transform) {
    ImGui::DragFloat3("Position", glm::value_ptr(transform.position), 0.1f);
    ImGui::DragFloat3("Rotation", glm::value_ptr(transform.rotation), 0.1f);
    ImGui::DragFloat3("Scale",    glm::value_ptr(transform.scale),    0.1f, 0.001f, 1000.0f);
}

void PropertiesPanel::drawMaterialComponent(MaterialComponent& material) {
    ImGui::ColorEdit3("Albedo", glm::value_ptr(material.albedo));
    ImGui::SliderFloat("Metallic",  &material.metallic,  0.0f, 1.0f);
    ImGui::SliderFloat("Roughness", &material.roughness, 0.0f, 1.0f);
}

void PropertiesPanel::drawLightComponent(LightComponent& light) {
    // Light type combo
    const char* lightTypes[] = { "Directional", "Point", "Spot" };
    int currentType = static_cast<int>(light.type);
    if (ImGui::Combo("Type", &currentType, lightTypes, IM_ARRAYSIZE(lightTypes))) {
        light.type = static_cast<LightType>(currentType);
    }

    ImGui::ColorEdit3("Color", glm::value_ptr(light.color));
    ImGui::DragFloat("Intensity", &light.intensity, 0.1f, 0.0f, 100.0f);

    // Show spot light specific parameters
    if (light.type == LightType::Spot) {
        ImGui::DragFloat("Inner Cone", &light.innerConeAngle, 0.1f, 0.0f, light.outerConeAngle);
        ImGui::DragFloat("Outer Cone", &light.outerConeAngle, 0.1f, light.innerConeAngle, 90.0f);
    }
}

} // namespace PixellentModeler
