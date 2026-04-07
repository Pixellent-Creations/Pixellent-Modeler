#include "PixellentModeler/Tools/MoveTool.hpp"
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
    if (!ctx.camera) return;

    // Update gizmo position to the selection centroid
    // (For now, use the transform of the primary selected entity)
    m_gizmo.setMode(GizmoMode::Translate);

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

    // Update the previous ray for the next frame
    m_prevRayOrigin = rayOrigin;
    m_prevRayDir = rayDir;

    // TODO: Apply the translation to selected entities' transforms once
    // the Transform component is implemented.
}

void MoveTool::onMouseUp(const ToolContext& ctx, int button, int mods) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;

    if (m_dragging) {
        // TODO: Create a TransformCommand and push it to ctx.commands
        // so the move can be undone/redone.
        // glm::vec3 totalDelta = m_gizmo.position() - m_originalPosition;
    }

    m_dragging = false;
    m_activeAxis = -1;
    (void)ctx;
    (void)mods;
}

void MoveTool::onRender(const ToolContext& ctx) {
    if (!ctx.selection || ctx.selection->current().isEmpty()) return;

    // TODO: Render the translate gizmo (3 colored arrows: red=X, green=Y, blue=Z)
    // at m_gizmo.position() using line rendering.
    // This requires a line-drawing utility or immediate-mode GL rendering,
    // which will be added when the Renderer supports debug line drawing.
    (void)ctx;
}

} // namespace PixellentModeler
