#pragma once

#include <glm/glm.hpp>

namespace PixellentModeler {

enum class GizmoMode { Translate, Rotate, Scale };
enum class GizmoAxis { None = -1, X = 0, Y = 1, Z = 2 };

class Gizmo {
public:
    void setPosition(const glm::vec3& pos) { m_position = pos; }
    void setMode(GizmoMode mode) { m_mode = mode; }

    GizmoAxis pickAxis(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                       const glm::mat4& view, const glm::mat4& proj,
                       const glm::vec2& viewportSize) const;

    glm::vec3 computeTranslateDrag(GizmoAxis axis,
                                    const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                                    const glm::vec3& prevRayOrigin, const glm::vec3& prevRayDir) const;

    float computeRotateDrag(GizmoAxis axis,
                            const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                            const glm::vec3& prevRayOrigin, const glm::vec3& prevRayDir) const;

    float computeScaleDrag(GizmoAxis axis,
                           const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                           const glm::vec3& prevRayOrigin, const glm::vec3& prevRayDir) const;

    const glm::vec3& position() const { return m_position; }
    GizmoMode mode() const { return m_mode; }

private:
    glm::vec3 m_position{0};
    GizmoMode m_mode = GizmoMode::Translate;
    float m_size = 1.0f;

    glm::vec3 closestPointOnAxis(GizmoAxis axis, const glm::vec3& rayOrigin,
                                  const glm::vec3& rayDir) const;

    glm::vec3 axisDirection(GizmoAxis axis) const;

    glm::vec3 projectToScreen(const glm::vec3& worldPos, const glm::mat4& viewProj,
                               const glm::vec2& viewportSize) const;
};

} // namespace PixellentModeler
