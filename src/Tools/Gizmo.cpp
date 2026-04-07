#include "PixellentModeler/Tools/Gizmo.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>

namespace PixellentModeler {

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

glm::vec3 Gizmo::axisDirection(GizmoAxis axis) const {
    switch (axis) {
        case GizmoAxis::X: return glm::vec3(1.0f, 0.0f, 0.0f);
        case GizmoAxis::Y: return glm::vec3(0.0f, 1.0f, 0.0f);
        case GizmoAxis::Z: return glm::vec3(0.0f, 0.0f, 1.0f);
        default:           return glm::vec3(0.0f);
    }
}

glm::vec3 Gizmo::projectToScreen(const glm::vec3& worldPos, const glm::mat4& viewProj,
                                   const glm::vec2& viewportSize) const {
    glm::vec4 clip = viewProj * glm::vec4(worldPos, 1.0f);
    if (std::abs(clip.w) < 1e-6f) {
        return glm::vec3(-1.0f); // behind camera
    }
    glm::vec3 ndc = glm::vec3(clip) / clip.w;
    float screenX = (ndc.x * 0.5f + 0.5f) * viewportSize.x;
    float screenY = (1.0f - (ndc.y * 0.5f + 0.5f)) * viewportSize.y;
    return glm::vec3(screenX, screenY, ndc.z);
}

glm::vec3 Gizmo::closestPointOnAxis(GizmoAxis axis, const glm::vec3& rayOrigin,
                                      const glm::vec3& rayDir) const {
    // Find the closest point on the axis line (m_position + t * axisDir) to the ray.
    // This is a closest-point-between-two-lines problem.
    glm::vec3 axisDir = axisDirection(axis);
    glm::vec3 w0 = m_position - rayOrigin;
    float a = glm::dot(axisDir, axisDir);  // always 1.0 for unit axis
    float b = glm::dot(axisDir, rayDir);
    float c = glm::dot(rayDir, rayDir);
    float d = glm::dot(axisDir, w0);
    float e = glm::dot(rayDir, w0);

    float denom = a * c - b * b;
    if (std::abs(denom) < 1e-6f) {
        // Lines are nearly parallel; return the gizmo position
        return m_position;
    }

    float t = (b * e - c * d) / denom;
    return m_position + axisDir * t;
}

// ---------------------------------------------------------------------------
// pickAxis
// ---------------------------------------------------------------------------

GizmoAxis Gizmo::pickAxis(const glm::vec3& /*rayOrigin*/, const glm::vec3& /*rayDir*/,
                           const glm::mat4& view, const glm::mat4& proj,
                           const glm::vec2& viewportSize) const {
    // Project each axis endpoint to screen space and compute distance from
    // the mouse position (assumed to be at the center of viewportSize for
    // the initial pick -- the caller should provide the mouse position via
    // the ray, but we use screen-space distance for robustness).

    glm::mat4 viewProj = proj * view;
    glm::vec3 originScreen = projectToScreen(m_position, viewProj, viewportSize);

    constexpr float kPickThreshold = 10.0f; // pixels
    float bestDist = kPickThreshold;
    GizmoAxis bestAxis = GizmoAxis::None;

    for (int i = 0; i < 3; ++i) {
        GizmoAxis axis = static_cast<GizmoAxis>(i);
        glm::vec3 endWorld = m_position + axisDirection(axis) * m_size;
        glm::vec3 endScreen = projectToScreen(endWorld, viewProj, viewportSize);

        // Compute the minimum distance from the mouse (viewport center) to the
        // screen-space line segment [originScreen, endScreen].
        // For simplicity, we compute distance from each endpoint and the midpoint.
        glm::vec2 midScreen = glm::vec2(originScreen + endScreen) * 0.5f;
        glm::vec2 mouse = viewportSize * 0.5f; // placeholder -- caller should use ray

        // Point-to-segment distance in 2D
        glm::vec2 a2d(originScreen.x, originScreen.y);
        glm::vec2 b2d(endScreen.x, endScreen.y);
        glm::vec2 ab = b2d - a2d;
        float abLen2 = glm::dot(ab, ab);
        float dist;
        if (abLen2 < 1e-6f) {
            dist = glm::length(mouse - a2d);
        } else {
            float t = glm::clamp(glm::dot(mouse - a2d, ab) / abLen2, 0.0f, 1.0f);
            glm::vec2 closest = a2d + ab * t;
            dist = glm::length(mouse - closest);
        }

        if (dist < bestDist) {
            bestDist = dist;
            bestAxis = axis;
        }
    }

    return bestAxis;
}

// ---------------------------------------------------------------------------
// computeTranslateDrag
// ---------------------------------------------------------------------------

glm::vec3 Gizmo::computeTranslateDrag(GizmoAxis axis,
                                        const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                                        const glm::vec3& prevRayOrigin, const glm::vec3& prevRayDir) const {
    glm::vec3 currentPoint = closestPointOnAxis(axis, rayOrigin, rayDir);
    glm::vec3 prevPoint = closestPointOnAxis(axis, prevRayOrigin, prevRayDir);
    glm::vec3 delta = currentPoint - prevPoint;

    // Constrain to the axis
    glm::vec3 dir = axisDirection(axis);
    return dir * glm::dot(delta, dir);
}

// ---------------------------------------------------------------------------
// computeRotateDrag
// ---------------------------------------------------------------------------

float Gizmo::computeRotateDrag(GizmoAxis axis,
                                const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                                const glm::vec3& prevRayOrigin, const glm::vec3& prevRayDir) const {
    // Intersect both rays with the plane perpendicular to the rotation axis
    // passing through the gizmo position.
    glm::vec3 normal = axisDirection(axis);
    float denom = glm::dot(normal, rayDir);
    float prevDenom = glm::dot(normal, prevRayDir);

    if (std::abs(denom) < 1e-6f || std::abs(prevDenom) < 1e-6f) {
        return 0.0f; // Ray is nearly parallel to the plane
    }

    float t = glm::dot(m_position - rayOrigin, normal) / denom;
    float prevT = glm::dot(m_position - prevRayOrigin, normal) / prevDenom;

    glm::vec3 hitPoint = rayOrigin + rayDir * t;
    glm::vec3 prevHitPoint = prevRayOrigin + prevRayDir * prevT;

    // Compute vectors from the gizmo center to each hit point
    glm::vec3 toCurrent = glm::normalize(hitPoint - m_position);
    glm::vec3 toPrev = glm::normalize(prevHitPoint - m_position);

    // Compute the signed angle between them
    float cosAngle = glm::clamp(glm::dot(toPrev, toCurrent), -1.0f, 1.0f);
    float angle = std::acos(cosAngle);

    // Determine sign using the cross product
    glm::vec3 cross = glm::cross(toPrev, toCurrent);
    if (glm::dot(cross, normal) < 0.0f) {
        angle = -angle;
    }

    return angle;
}

// ---------------------------------------------------------------------------
// computeScaleDrag
// ---------------------------------------------------------------------------

float Gizmo::computeScaleDrag(GizmoAxis axis,
                               const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                               const glm::vec3& prevRayOrigin, const glm::vec3& prevRayDir) const {
    // Project the drag distance along the axis and convert to a scale factor
    glm::vec3 currentPoint = closestPointOnAxis(axis, rayOrigin, rayDir);
    glm::vec3 prevPoint = closestPointOnAxis(axis, prevRayOrigin, prevRayDir);

    glm::vec3 dir = axisDirection(axis);
    float currentDist = glm::dot(currentPoint - m_position, dir);
    float prevDist = glm::dot(prevPoint - m_position, dir);

    // Avoid division by zero
    if (std::abs(prevDist) < 1e-6f) {
        return 1.0f;
    }

    return currentDist / prevDist;
}

} // namespace PixellentModeler
