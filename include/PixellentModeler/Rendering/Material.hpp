#pragma once

#include <memory>
#include <glm/glm.hpp>

namespace PixellentModeler {

class Texture;

struct Material {
    glm::vec3 albedo{0.8f, 0.8f, 0.8f};
    float metallic = 0.0f;
    float roughness = 0.5f;

    std::shared_ptr<Texture> albedoMap;
    std::shared_ptr<Texture> normalMap;
    std::shared_ptr<Texture> metallicMap;
    std::shared_ptr<Texture> roughnessMap;
};

} // namespace PixellentModeler
