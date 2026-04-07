#include "PixellentModeler/Tools/MoveTool.hpp"
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
    // Convert mouse position to normalized device coordinates
    float x = (2.0f * mousePos.x) / viewportSize.x - 1.0f;
    float y = 1.0f - (2.0f * mousePos.y) / viewportSize.y;
    glm::vec4 clipCoords(x, y, -1.0f, 1.0f);

    // Unproject to view space
    glm::vec4 eyeCoords = invProj * clipCoords;
    eyeCoords = glm::vec4(eyeCoords.x, eyeCoords.y, -1.0f, 0.0f);

    // Unproject to world space
    glm::vec3 worldRay = glm::vec3(invView * eyeCoords);
    return glm::normalize(worldRay);
}

static glm::vec3 getCameraOrigin(const glm::mat4& invView) {
    return glm::vec3(invView * glm::vec4(0, 0, 0, 1));
}

static float snapValue(float value, float snapSize) {
    return std::round(value / snapSize) * snapSize;
}

// ---------------------------------------------------------------------------
// MoveTool
// ---------------------------------------------------------------------------

void MoveTool::onMouseDown(const ToolContext& ctx, int button, int mods) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    if (!ctx.selection || ctx.selection->current().isEmpty()) return;
    if (!ctx.scene || !ctx.camera) return;

    // Update gizmo position to the selection centroid
    m_gizmo.setMode(GizmoMode::Translate);
    glm::vec3 center = ctx.selection->selectionCenter(*ctx.scene);
    m_gizmo.setPosition(center);

    // Compute ray from mouse position
    glm::mat4 invProj = glm::inverse(ctx.camera->projection());
    glm::mat4 invView = glm::inverse(ctx.camera->view());
    glm::vec3 rayDir = screenToRay(ctx.mousePos, ctx.viewportSize, invProj, invView);
    glm::vec3 rayOrigin = getCameraOrigin(invView);

    // Check if mouse hits a gizmo axis
    GizmoAxis axis = m_gizmo.pickAxis(rayOrigin, rayDir,
                                       ctx.camera->view(), ctx.camera->projection(),
                                       ctx.viewportSize);
    if (axis == GizmoAxis::None) return;

    m_activeAxis = static_cast<int>(axis);
    m_dragging = true;
    m_originalPosition = m_gizmo.position();
    m_dragStart = glm::vec3(ctx.mousePos, 0.0f); // store mouse pos for delta
    m_snapping = (mods & GLFW_MOD_CONTROL) != 0;

    // Store the initial ray for later delta computation
    m_prevRayOrigin = rayOrigin;
    m_prevRayDir = rayDir;
}

void MoveTool::onMouseMove(const ToolContext& ctx) {
    if (!m_dragging || m_activeAxis < 0) return;
    if (!ctx.camera) return;

    glm::mat4 invProj = glm::inverse(ctx.camera->projection());
    glm::mat4 invView = glm::inverse(ctx.camera->view());
    glm::vec3 rayDir = screenToRay(ctx.mousePos, ctx.viewportSize, invProj, invView);
    glm::vec3 rayOrigin = getCameraOrigin(invView);

    GizmoAxis axis = static_cast<GizmoAxis>(m_activeAxis);

    // Compute delta translation along the constrained axis using the Gizmo
    glm::vec3 delta = m_gizmo.computeTranslateDrag(axis,
                                                     rayOrigin, rayDir,
                                                     m_prevRayOrigin, m_prevRayDir);

    // Apply snapping if enabled
    if (m_snapping) {
        glm::vec3 axisDir(0.0f);
        axisDir[m_activeAxis] = 1.0f;
        float dist = glm::dot(delta, axisDir);
        dist = snapValue(dist, m_snapSize);
        delta = axisDir * dist;
    }

    glm::vec3 newPos = m_gizmo.position() + delta;
    m_gizmo.setPosition(newPos);

    // Apply translation to all selected entities
    if (ctx.scene && ctx.selection) {
        const auto& selectedEntities = ctx.selection->current().entities;
        for (EntityID entityId : selectedEntities) {
            Entity* entity = ctx.scene->getEntity(entityId);
            if (!entity) continue;

            auto* tc = entity->getComponent<TransformComponent>();
            if (!tc) continue;

            tc->position += delta;
        }
    }

    // Update the previous ray for the next frame
    m_prevRayOrigin = rayOrigin;
    m_prevRayDir = rayDir;
}

void MoveTool::onMouseUp(const ToolContext& ctx, int button, int mods) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;

    if (m_dragging && ctx.scene && ctx.selection && ctx.commands) {
        // Calculate total delta from move
        glm::vec3 totalDelta = m_gizmo.position() - m_originalPosition;

        // Only create command if there was actual movement
        if (glm::length(totalDelta) > 0.001f) {
            std::vector<TransformCommand::EntityTransform> transforms;

            // Get all selected entities and store their transforms
            const auto& selectedEntities = ctx.selection->current().entities;
            for (EntityID entityId : selectedEntities) {
                Entity* entity = ctx.scene->getEntity(entityId);
                if (!entity) continue;

                auto* tc = entity->getComponent<TransformComponent>();
                if (!tc) continue;

                TransformCommand::EntityTransform t;
                t.id = entityId;
                t.oldPosition = tc->position - totalDelta;  // original before this operation
                t.oldRotation = tc->rotation;
                t.oldScale = tc->scale;
                t.newPosition = tc->position;  // current (already updated during drag)
                t.newRotation = tc->rotation;
                t.newScale = tc->scale;

                transforms.push_back(t);
            }

            if (!transforms.empty()) {
                auto cmd = std::make_shared<TransformCommand>(ctx.scene, transforms);
                ctx.commands->execute(cmd);
            }
        }
    }

    m_dragging = false;
    m_activeAxis = -1;
    (void)mods;
}

void MoveTool::onRender(const ToolContext& ctx) {
    if (!ctx.selection || ctx.selection->current().isEmpty()) return;
    if (!ctx.renderer) return;

    // Render the translate gizmo
    ctx.renderer->drawGizmo(m_gizmo.position(), static_cast<int>(GizmoMode::Translate), m_activeAxis);
}

} // namespace PixellentModeler
