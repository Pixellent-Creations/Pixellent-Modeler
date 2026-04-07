#include "PixellentModeler/Tools/RotateTool.hpp"
#include "PixellentModeler/Selection/SelectionManager.hpp"
#include "PixellentModeler/Scene/Camera.hpp"
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
    if (!ctx.camera) return;

    m_gizmo.setMode(GizmoMode::Rotate);

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
    m_originalRotation = glm::vec3(0.0f); // TODO: read from entity transform
    m_snapping = (mods & GLFW_MOD_CONTROL) != 0;
    m_prevMousePos = ctx.mousePos;
}

void RotateTool::onMouseMove(const ToolContext& ctx) {
    if (!m_dragging || m_activeAxis < 0) return;
    if (!ctx.camera) return;

    glm::mat4 invProj = glm::inverse(ctx.camera->projection());
    glm::mat4 invView = glm::inverse(ctx.camera->view());
    glm::vec3 rayDir = screenToRay(ctx.mousePos, ctx.viewportSize, invProj, invView);
    glm::vec3 rayOrigin = glm::vec3(invView * glm::vec4(0, 0, 0, 1));

    // Use a simplified approach: map horizontal mouse delta to rotation angle
    // A more precise implementation would project onto the rotation circle.
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

    // TODO: Apply rotation to selected entities' transforms once
    // the Transform component is implemented.
    // The rotation should be around the axis defined by m_activeAxis,
    // centered at the gizmo position.
    (void)axis;
}

void RotateTool::onMouseUp(const ToolContext& ctx, int button, int mods) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;

    if (m_dragging) {
        // TODO: Create a TransformCommand for undo/redo
    }

    m_dragging = false;
    m_activeAxis = -1;
    m_totalAngle = 0.0f;
    (void)ctx;
    (void)mods;
}

void RotateTool::onRender(const ToolContext& ctx) {
    if (!ctx.selection || ctx.selection->current().isEmpty()) return;

    // TODO: Render rotation circles at the gizmo position.
    // Three circles: red around X axis, green around Y axis, blue around Z axis.
    // Highlight the active axis during a drag.
    (void)ctx;
}

} // namespace PixellentModeler
