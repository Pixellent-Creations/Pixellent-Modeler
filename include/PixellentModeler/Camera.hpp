#pragma once

#include <glm/glm.hpp>

namespace PixellentModeler {

class Camera {
public:
    Camera() = default;
    Camera(float fovDegrees, float aspectRatio, float nearPlane = 0.1f, float farPlane = 100.0f);

    // position/orientation
    void setPosition(const glm::vec3& pos) { m_position = pos; updateView(); }
    void lookAt(const glm::vec3& target, const glm::vec3& up = glm::vec3(0,1,0));

    const glm::mat4& view() const { return m_view; }
    const glm::mat4& projection() const { return m_projection; }

    void setAspectRatio(float aspect);

private:
    void updateView();

    glm::vec3 m_position{0,0,5};
    glm::vec3 m_target{0,0,0};
    glm::vec3 m_up{0,1,0};

    glm::mat4 m_view{1.0f};
    glm::mat4 m_projection{1.0f};

    float m_fov = 45.0f;
    float m_aspect = 4.0f/3.0f;
    float m_near = 0.1f;
    float m_far = 100.0f;
};

} // namespace PixellentModeler
