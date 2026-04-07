#pragma once

#include <glm/glm.hpp>

namespace PixellentModeler {

enum class LightType { Directional, Point, Spot };

struct LightData {
    LightType type = LightType::Directional;
    glm::vec3 position{0.0f, 10.0f, 0.0f};
    glm::vec3 direction{0.0f, -1.0f, 0.0f};
    glm::vec3 color{1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
};

} // namespace PixellentModeler
