#include "PixellentModeler/Scene/Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

namespace PixellentModeler {

Camera::Camera() {
    updatePositionFromAngles();
    updateView();
    updateProjection();
}

Camera::Camera(float fovDegrees, float aspectRatio, float nearPlane, float farPlane)
    : m_fov(fovDegrees), m_aspect(aspectRatio), m_near(nearPlane), m_far(farPlane) {
    updatePositionFromAngles();
    updateView();
    updateProjection();
}

void Camera::orbit(float deltaX, float deltaY) {
    m_yaw += deltaX;
    m_pitch += deltaY;

    // Clamp pitch to avoid gimbal lock
    m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);

    updatePositionFromAngles();
    updateView();
}

void Camera::pan(float deltaX, float deltaY) {
    // Compute right and up vectors relative to the camera
    glm::vec3 forward = glm::normalize(m_target - m_position);
    glm::vec3 right = glm::normalize(glm::cross(forward, m_up));
    glm::vec3 up = glm::normalize(glm::cross(right, forward));

    // Scale pan speed by distance for consistent feel
    float panSpeed = m_distance * 0.002f;
    glm::vec3 offset = right * (-deltaX * panSpeed) + up * (deltaY * panSpeed);

    m_target += offset;
    m_position += offset;

    updateView();
}

void Camera::zoom(float delta) {
    m_distance *= (1.0f - delta * 0.1f);
    m_distance = std::max(m_distance, 0.1f);

    updatePositionFromAngles();
    updateView();

    // Also update ortho size for orthographic mode
    m_orthoSize = m_distance;
    if (!m_perspective) {
        updateProjection();
    }
}

void Camera::setPosition(const glm::vec3& pos) {
    m_position = pos;

    // Recalculate distance and angles from the new position
    glm::vec3 diff = m_position - m_target;
    m_distance = glm::length(diff);
    if (m_distance > 0.0001f) {
        m_pitch = glm::degrees(std::asin(diff.y / m_distance));
        m_yaw = glm::degrees(std::atan2(diff.z, diff.x));
    }

    updateView();
}

void Camera::lookAt(const glm::vec3& target, const glm::vec3& up) {
    m_target = target;
    m_up = up;

    // Recalculate distance and angles
    glm::vec3 diff = m_position - m_target;
    m_distance = glm::length(diff);
    if (m_distance > 0.0001f) {
        m_pitch = glm::degrees(std::asin(diff.y / m_distance));
        m_yaw = glm::degrees(std::atan2(diff.z, diff.x));
    }

    updateView();
}

void Camera::setAspectRatio(float aspect) {
    m_aspect = aspect;
    updateProjection();
}

void Camera::setFOV(float fovDegrees) {
    m_fov = fovDegrees;
    updateProjection();
}

void Camera::setPerspective(bool perspective) {
    m_perspective = perspective;
    updateProjection();
}

void Camera::setFrontView() {
    m_yaw = -90.0f;
    m_pitch = 0.0f;
    updatePositionFromAngles();
    updateView();
}

void Camera::setRightView() {
    m_yaw = 0.0f;
    m_pitch = 0.0f;
    updatePositionFromAngles();
    updateView();
}

void Camera::setTopView() {
    m_yaw = -90.0f;
    m_pitch = 89.9f;
    updatePositionFromAngles();
    updateView();
}

void Camera::resetView() {
    m_target = glm::vec3(0.0f, 0.0f, 0.0f);
    m_yaw = -90.0f;
    m_pitch = 25.0f;
    m_distance = 11.18f;
    updatePositionFromAngles();
    updateView();
}

glm::vec3 Camera::screenToWorldRay(float screenX, float screenY, float viewportW, float viewportH) const {
    // Convert screen coordinates to normalized device coordinates
    float ndcX = (2.0f * screenX) / viewportW - 1.0f;
    float ndcY = 1.0f - (2.0f * screenY) / viewportH;

    // Create clip-space coordinates
    glm::vec4 clipCoords(ndcX, ndcY, -1.0f, 1.0f);

    // Unproject to eye space
    glm::mat4 invProjection = glm::inverse(m_projection);
    glm::vec4 eyeCoords = invProjection * clipCoords;
    eyeCoords = glm::vec4(eyeCoords.x, eyeCoords.y, -1.0f, 0.0f);

    // Unproject to world space
    glm::mat4 invView = glm::inverse(m_view);
    glm::vec4 worldRay = invView * eyeCoords;

    return glm::normalize(glm::vec3(worldRay));
}

void Camera::updateView() {
    m_view = glm::lookAt(m_position, m_target, m_up);
}

void Camera::updateProjection() {
    if (m_perspective) {
        m_projection = glm::perspective(glm::radians(m_fov), m_aspect, m_near, m_far);
    } else {
        float halfW = m_orthoSize * m_aspect * 0.5f;
        float halfH = m_orthoSize * 0.5f;
        m_projection = glm::ortho(-halfW, halfW, -halfH, halfH, m_near, m_far);
    }
}

void Camera::updatePositionFromAngles() {
    float yawRad = glm::radians(m_yaw);
    float pitchRad = glm::radians(m_pitch);

    glm::vec3 offset;
    offset.x = m_distance * std::cos(pitchRad) * std::cos(yawRad);
    offset.y = m_distance * std::sin(pitchRad);
    offset.z = m_distance * std::cos(pitchRad) * std::sin(yawRad);

    m_position = m_target + offset;
}

} // namespace PixellentModeler
