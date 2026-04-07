#pragma once
#include "PixellentModeler/Scene/Component.hpp"
#include "PixellentModeler/Rendering/Material.hpp"
#include <glm/glm.hpp>

namespace PixellentModeler {

class MaterialComponent : public Component {
public:
    // Direct fields for easy UI/serialization access
    glm::vec3 albedo{0.8f, 0.8f, 0.8f};
    float metallic = 0.0f;
    float roughness = 0.5f;

    // Convert to a Material struct for use with the Renderer
    Material toMaterial() const {
        Material m;
        m.albedo = albedo;
        m.metallic = metallic;
        m.roughness = roughness;
        return m;
    }
};

} // namespace PixellentModeler
