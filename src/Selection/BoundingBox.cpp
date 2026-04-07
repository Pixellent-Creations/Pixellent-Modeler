#include "PixellentModeler/Selection/BoundingBox.hpp"

#include <algorithm>
#include <cmath>

namespace PixellentModeler {

void BoundingBox::expand(const glm::vec3& point) {
    min = glm::min(min, point);
    max = glm::max(max, point);
}

void BoundingBox::expand(const BoundingBox& other) {
    if (!other.isValid())
        return;
    min = glm::min(min, other.min);
    max = glm::max(max, other.max);
}

glm::vec3 BoundingBox::center() const {
    return (min + max) * 0.5f;
}

glm::vec3 BoundingBox::size() const {
    return max - min;
}

bool BoundingBox::contains(const glm::vec3& point) const {
    return point.x >= min.x && point.x <= max.x &&
           point.y >= min.y && point.y <= max.y &&
           point.z >= min.z && point.z <= max.z;
}

bool BoundingBox::intersectsRay(const glm::vec3& origin, const glm::vec3& direction, float& tMin) const {
    // Slab method for ray-AABB intersection
    float tNear = -FLT_MAX;
    float tFar  =  FLT_MAX;

    for (int i = 0; i < 3; ++i) {
        if (std::abs(direction[i]) < 1e-8f) {
            // Ray is parallel to the slab planes for this axis
            if (origin[i] < min[i] || origin[i] > max[i])
                return false;
        } else {
            float invD = 1.0f / direction[i];
            float t1 = (min[i] - origin[i]) * invD;
            float t2 = (max[i] - origin[i]) * invD;

            if (t1 > t2) std::swap(t1, t2);

            tNear = std::max(tNear, t1);
            tFar  = std::min(tFar, t2);

            if (tNear > tFar)
                return false;
        }
    }

    // Intersection behind the ray origin
    if (tFar < 0.0f)
        return false;

    tMin = (tNear >= 0.0f) ? tNear : tFar;
    return true;
}

bool BoundingBox::isValid() const {
    return min.x <= max.x && min.y <= max.y && min.z <= max.z;
}

} // namespace PixellentModeler
