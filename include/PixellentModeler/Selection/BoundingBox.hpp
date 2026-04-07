#pragma once

#include <cfloat>
#include <glm/glm.hpp>

namespace PixellentModeler {

struct BoundingBox {
    glm::vec3 min{FLT_MAX};
    glm::vec3 max{-FLT_MAX};

    void expand(const glm::vec3& point);
    void expand(const BoundingBox& other);
    glm::vec3 center() const;
    glm::vec3 size() const;
    bool contains(const glm::vec3& point) const;
    bool intersectsRay(const glm::vec3& origin, const glm::vec3& direction, float& tMin) const;
    bool isValid() const;
};

} // namespace PixellentModeler
