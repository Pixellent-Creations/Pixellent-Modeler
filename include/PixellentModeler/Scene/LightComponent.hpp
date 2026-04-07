#pragma once
#include "PixellentModeler/Scene/Component.hpp"
#include "PixellentModeler/Rendering/Light.hpp"
#include <glm/glm.hpp>

namespace PixellentModeler {

class LightComponent : public Component {
public:
    LightType type = LightType::Directional;
    glm::vec3 color{1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
    glm::vec3 direction{0.0f, -1.0f, 0.0f};

    // Spot light parameters (reserved for future use)
    float innerConeAngle = 12.5f;
    float outerConeAngle = 17.5f;

    // Convert to a LightData struct for use with the Renderer
    LightData toLightData() const {
        LightData ld;
        ld.type = type;
        ld.color = color;
        ld.intensity = intensity;
        ld.direction = direction;
        return ld;
    }
};

} // namespace PixellentModeler
