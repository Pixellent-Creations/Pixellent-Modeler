#pragma once
#include <glm/glm.hpp>

namespace PixellentModeler {

class Camera {
public:
    Camera();
    Camera(float fovDegrees, float aspectRatio, float nearPlane = 0.1f, float farPlane = 100.0f);

    // Orbit camera controls
    void orbit(float deltaX, float deltaY);   // rotate around target
    void pan(float deltaX, float deltaY);     // move target and position
    void zoom(float delta);                    // dolly toward/away from target

    void setPosition(const glm::vec3& pos);
    void setTarget(const glm::vec3& target) { m_target = target; updateView(); }
    void lookAt(const glm::vec3& target, const glm::vec3& up = glm::vec3(0, 1, 0));
    void setAspectRatio(float aspect);
    void setFOV(float fovDegrees);
    void setPerspective(bool perspective);

    // Preset views
    void setFrontView();
    void setRightView();
    void setTopView();
    void resetView();

    const glm::mat4& view() const { return m_view; }
    const glm::mat4& projection() const { return m_projection; }
    const glm::vec3& position() const { return m_position; }
    const glm::vec3& target() const { return m_target; }
    float distance() const { return m_distance; }
    bool isPerspective() const { return m_perspective; }

    // Convenience aliases used by serialization
    const glm::vec3& getPosition() const { return m_position; }
    const glm::vec3& getTarget() const { return m_target; }
    float getFov() const { return m_fov; }
    float getDistance() const { return m_distance; }
    void setFov(float fovDegrees) { setFOV(fovDegrees); }
    void setDistance(float dist) { m_distance = dist; updatePositionFromAngles(); }

    // Ray casting for picking
    glm::vec3 screenToWorldRay(float screenX, float screenY, float viewportW, float viewportH) const;

private:
    void updateView();
    void updateProjection();
    void updatePositionFromAngles();

    glm::vec3 m_position{0, 5, 10};
    glm::vec3 m_target{0, 0, 0};
    glm::vec3 m_up{0, 1, 0};

    float m_yaw = -90.0f;     // horizontal angle (degrees)
    float m_pitch = 25.0f;     // vertical angle (degrees)
    float m_distance = 11.18f; // distance from target

    glm::mat4 m_view{1.0f};
    glm::mat4 m_projection{1.0f};

    float m_fov = 45.0f;
    float m_aspect = 16.0f / 9.0f;
    float m_near = 0.1f;
    float m_far = 1000.0f;
    bool m_perspective = true;
    float m_orthoSize = 10.0f;  // for orthographic mode
};

} // namespace PixellentModeler
