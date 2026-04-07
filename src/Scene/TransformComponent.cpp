#include "PixellentModeler/Scene/TransformComponent.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace PixellentModeler {

glm::mat4 TransformComponent::modelMatrix() const {
    glm::mat4 model{1.0f};

    // Translate
    model = glm::translate(model, position);

    // Rotate Z * Y * X (applied right-to-left: X first, then Y, then Z)
    model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));

    // Scale
    model = glm::scale(model, scale);

    return model;
}

} // namespace PixellentModeler
