#include "PixellentModeler/Tools/RotateTool.hpp"
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

static float snapAngle(float angleDeg, float snapDeg) {
    return std::round(angleDeg / snapDeg) * snapDeg;
}

// ---------------------------------------------------------------------------
// RotateTool
// ---------------------------------------------------------------------------

void RotateTool::onMouseDown(const ToolContext& ctx, int button, int mods) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    if (!ctx.selection || ctx.selection->current().isEmpty()) return;
    if (!ctx.scene || !ctx.camera) return;

    m_gizmo.setMode(GizmoMode::Rotate);
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
    m_totalAngle = 0.0f;

    // Store original rotations from selected entities
    const auto& selectedEntities = ctx.selection->current().entities;
    if (!selectedEntities.empty()) {
        EntityID firstId = *selectedEntities.begin();
        Entity* first = ctx.scene->getEntity(firstId);
        if (first) {
            auto* tc = first->getComponent<TransformComponent>();
            if (tc) {
                m_originalRotation = tc->rotation;
            }
        }
    }

    m_snapping = (mods & GLFW_MOD_CONTROL) != 0;
    m_prevMousePos = ctx.mousePos;
}

void RotateTool::onMouseMove(const ToolContext& ctx) {
    if (!m_dragging || m_activeAxis < 0) return;
    if (!ctx.camera || !ctx.scene || !ctx.selection) return;

    glm::mat4 invProj = glm::inverse(ctx.camera->projection());
    glm::mat4 invView = glm::inverse(ctx.camera->view());
    glm::vec3 rayDir = screenToRay(ctx.mousePos, ctx.viewportSize, invProj, invView);
    glm::vec3 rayOrigin = glm::vec3(invView * glm::vec4(0, 0, 0, 1));

    GizmoAxis axis = static_cast<GizmoAxis>(m_activeAxis);

    // Compute the drag angle from horizontal mouse delta
    float deltaX = ctx.mousePos.x - m_prevMousePos.x;
    m_prevMousePos = ctx.mousePos;

    float angleDelta = deltaX * 0.5f; // degrees per pixel

    if (m_snapping) {
        m_totalAngle += angleDelta;
        float snapped = snapAngle(m_totalAngle, m_snapAngle);
        angleDelta = snapped - (m_totalAngle - angleDelta);
        m_totalAngle = snapped;
    } else {
        m_totalAngle += angleDelta;
    }

    // Apply rotation to selected entities
    const auto& selectedEntities = ctx.selection->current().entities;
    for (EntityID entityId : selectedEntities) {
        Entity* entity = ctx.scene->getEntity(entityId);
        if (!entity) continue;

        auto* tc = entity->getComponent<TransformComponent>();
        if (!tc) continue;

        // Reset to original rotation first
        tc->rotation = m_originalRotation;

        // Apply cumulative rotation around the selected axis
        glm::vec3 rotation = glm::vec3(0.0f);
        rotation[m_activeAxis] = m_totalAngle;
        tc->rotation += rotation;
    }

    (void)axis;
}

void RotateTool::onMouseUp(const ToolContext& ctx, int button, int mods) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;

    if (m_dragging && ctx.scene && ctx.selection && ctx.commands && std::abs(m_totalAngle) > 0.1f) {
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
            t.oldRotation = m_originalRotation;
            t.oldScale = tc->scale;
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
    m_totalAngle = 0.0f;
    (void)mods;
}

void RotateTool::onRender(const ToolContext& ctx) {
    if (!ctx.selection || ctx.selection->current().isEmpty()) return;
    if (!ctx.renderer) return;

    ctx.renderer->drawGizmo(m_gizmo.position(), static_cast<int>(GizmoMode::Rotate), m_activeAxis);
}

} // namespace PixellentModeler
