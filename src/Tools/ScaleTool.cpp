#include "PixellentModeler/Tools/ScaleTool.hpp"
#include "PixellentModeler/Selection/SelectionManager.hpp"
#include "PixellentModeler/Scene/Camera.hpp"
#include "PixellentModeler/Scene/Scene.hpp"
#include "PixellentModeler/Scene/Entity.hpp"
#include "PixellentModeler/Scene/TransformComponent.hpp"
#include "PixellentModeler/Core/TransformCommand.hpp"
#include "PixellentModeler/Core/CommandHistory.hpp"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

namespace PixellentModeler {

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static glm::vec3 screenToRay(const glm::vec2& mousePos, const glm::vec2& viewportSize,
                              const glm::mat4& invProj, const glm::mat4& invView) {
    float x = (2.0f * mousePos.x) / viewportSize.x - 1.0f;
    float y = 1.0f - (2.0f * mousePos.y) / viewportSize.y;
    glm::vec4 clipCoords(x, y, -1.0f, 1.0f);
    glm::vec4 eyeCoords = invProj * clipCoords;
    eyeCoords = glm::vec4(eyeCoords.x, eyeCoords.y, -1.0f, 0.0f);
    glm::vec3 worldRay = glm::vec3(invView * eyeCoords);
    return glm::normalize(worldRay);
}

static float snapValue(float value, float snapSize) {
    return std::round(value / snapSize) * snapSize;
}

// ---------------------------------------------------------------------------
// ScaleTool
// ---------------------------------------------------------------------------

void ScaleTool::onMouseDown(const ToolContext& ctx, int button, int mods) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    if (!ctx.selection || ctx.selection->current().isEmpty()) return;
    if (!ctx.scene || !ctx.camera) return;

    m_gizmo.setMode(GizmoMode::Scale);
    glm::vec3 center = ctx.selection->selectionCenter(*ctx.scene);
    m_gizmo.setPosition(center);

    glm::mat4 invProj = glm::inverse(ctx.camera->projection());
    glm::mat4 invView = glm::inverse(ctx.camera->view());
    glm::vec3 rayDir = screenToRay(ctx.mousePos, ctx.viewportSize, invProj, invView);
    glm::vec3 rayOrigin = glm::vec3(invView * glm::vec4(0, 0, 0, 1));

    GizmoAxis axis = m_gizmo.pickAxis(rayOrigin, rayDir,
                                       ctx.camera->view(), ctx.camera->projection(),
                                       ctx.viewportSize);
    if (axis == GizmoAxis::None) return;

    m_activeAxis = static_cast<int>(axis);
    m_dragging = true;
    m_scaleFactor = 1.0f;

    // Store original scales from selected entities
    const auto& selectedEntities = ctx.selection->current().entities;
    if (!selectedEntities.empty()) {
        EntityID firstId = *selectedEntities.begin();
        Entity* first = ctx.scene->getEntity(firstId);
        if (first) {
            auto* tc = first->getComponent<TransformComponent>();
            if (tc) {
                m_originalScale = tc->scale;
            }
        }
    }

    m_snapping = (mods & GLFW_MOD_CONTROL) != 0;
    m_prevMousePos = ctx.mousePos;
}

void ScaleTool::onMouseMove(const ToolContext& ctx) {
    if (!m_dragging || m_activeAxis < 0) return;
    if (!ctx.camera || !ctx.scene || !ctx.selection) return;

    glm::mat4 invProj = glm::inverse(ctx.camera->projection());
    glm::mat4 invView = glm::inverse(ctx.camera->view());
    glm::vec3 rayDir = screenToRay(ctx.mousePos, ctx.viewportSize, invProj, invView);
    glm::vec3 rayOrigin = glm::vec3(invView * glm::vec4(0, 0, 0, 1));

    GizmoAxis axis = static_cast<GizmoAxis>(m_activeAxis);

    // Use mouse Y delta as scale factor (dragging up = scale up, down = scale down)
    float deltaY = -(ctx.mousePos.y - m_prevMousePos.y); // negate so up = positive
    m_prevMousePos = ctx.mousePos;

    // Convert pixel delta to scale: 100px = 1x scale change
    float scaleDelta = deltaY / 100.0f;
    m_scaleFactor += scaleDelta;

    // Clamp to avoid zero or negative scale
    m_scaleFactor = std::max(m_scaleFactor, 0.01f);

    if (m_snapping) {
        m_scaleFactor = snapValue(m_scaleFactor, m_snapSize);
        m_scaleFactor = std::max(m_scaleFactor, m_snapSize);
    }

    // Apply scale to selected entities
    const auto& selectedEntities = ctx.selection->current().entities;
    for (EntityID entityId : selectedEntities) {
        Entity* entity = ctx.scene->getEntity(entityId);
        if (!entity) continue;

        auto* tc = entity->getComponent<TransformComponent>();
        if (!tc) continue;

        // Reset to original scale first
        tc->scale = m_originalScale;

        // Apply scale along the selected axis
        tc->scale[m_activeAxis] *= m_scaleFactor;
    }

    (void)rayOrigin;
    (void)rayDir;
    (void)axis;
}

void ScaleTool::onMouseUp(const ToolContext& ctx, int button, int mods) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;

    if (m_dragging && ctx.scene && ctx.selection && ctx.commands && std::abs(m_scaleFactor - 1.0f) > 0.01f) {
        std::vector<TransformCommand::EntityTransform> transforms;

        const auto& selectedEntities = ctx.selection->current().entities;
        for (EntityID entityId : selectedEntities) {
            Entity* entity = ctx.scene->getEntity(entityId);
            if (!entity) continue;

            auto* tc = entity->getComponent<TransformComponent>();
            if (!tc) continue;

            TransformCommand::EntityTransform t;
            t.id = entityId;
            t.oldPosition = tc->position;
            t.oldRotation = tc->rotation;
            t.oldScale = m_originalScale;
            t.newPosition = tc->position;
            t.newRotation = tc->rotation;
            t.newScale = tc->scale;

            transforms.push_back(t);
        }

        if (!transforms.empty()) {
            auto cmd = std::make_shared<TransformCommand>(ctx.scene, transforms);
            ctx.commands->execute(cmd);
        }
    }

    m_dragging = false;
    m_activeAxis = -1;
    m_scaleFactor = 1.0f;
    (void)mods;
}

void ScaleTool::onRender(const ToolContext& ctx) {
    if (!ctx.selection || ctx.selection->current().isEmpty()) return;
    if (!ctx.renderer) return;

    ctx.renderer->drawGizmo(m_gizmo.position(), static_cast<int>(GizmoMode::Scale), m_activeAxis);
}

} // namespace PixellentModeler
