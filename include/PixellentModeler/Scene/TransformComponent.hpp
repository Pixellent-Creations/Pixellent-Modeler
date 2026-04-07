#pragma once
#include "PixellentModeler/Scene/Component.hpp"
#include <glm/glm.hpp>

namespace PixellentModeler {

class TransformComponent : public Component {
public:
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};  // Euler angles in degrees
    glm::vec3 scale{1.0f};

    glm::mat4 modelMatrix() const;
};

} // namespace PixellentModeler
