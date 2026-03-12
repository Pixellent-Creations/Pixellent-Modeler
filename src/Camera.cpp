#include "PixellentModeler/Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace PixellentModeler {

Camera::Camera(float fovDegrees, float aspectRatio, float nearPlane, float farPlane)
    : m_fov(fovDegrees), m_aspect(aspectRatio), m_near(nearPlane), m_far(farPlane)
{
    m_projection = glm::perspective(glm::radians(m_fov), m_aspect, m_near, m_far);
    updateView();
}

void Camera::updateView() {
    m_view = glm::lookAt(m_position, m_target, m_up);
}

void Camera::lookAt(const glm::vec3& target, const glm::vec3& up) {
    m_target = target;
    m_up = up;
    updateView();
}

void Camera::setAspectRatio(float aspect) {
    m_aspect = aspect;
    m_projection = glm::perspective(glm::radians(m_fov), m_aspect, m_near, m_far);
}

} // namespace PixellentModeler
