#include "PixellentModeler/Mesh/UVMapping.hpp"
#include "PixellentModeler/Mesh/HalfEdgeMesh.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <vector>
#include <cmath>
#include <algorithm>
#include <limits>

namespace PixellentModeler {

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

// Compute the axis-aligned bounding box of a set of vertices
static void computeBounds(const std::vector<glm::vec3>& vertices,
                          glm::vec3& minBounds, glm::vec3& maxBounds) {
    minBounds = glm::vec3(std::numeric_limits<float>::max());
    maxBounds = glm::vec3(std::numeric_limits<float>::lowest());

    for (const auto& v : vertices) {
        minBounds = glm::min(minBounds, v);
        maxBounds = glm::max(maxBounds, v);
    }
}

// Safely normalize a value from [lo, hi] to [0, 1], avoiding division by zero
static float normalizeRange(float value, float lo, float hi) {
    float range = hi - lo;
    if (std::abs(range) < 1e-7f) {
        return 0.5f;
    }
    return (value - lo) / range;
}

// ---------------------------------------------------------------------------
// Planar projection
// ---------------------------------------------------------------------------

void UVMapping::projectPlanar(HalfEdgeMesh& mesh, int axis) {
    const auto& vertices = mesh.getVertices();
    if (vertices.empty()) return;

    glm::vec3 minB, maxB;
    computeBounds(vertices, minB, maxB);

    // Determine which two axes to map to U and V based on the projection axis
    // axis=0 (X): project from X, so U=Z, V=Y
    // axis=1 (Y): project from Y, so U=X, V=Z
    // axis=2 (Z): project from Z, so U=X, V=Y
    int uAxis, vAxis;
    switch (axis) {
        case 0: uAxis = 2; vAxis = 1; break; // project from X
        case 2: uAxis = 0; vAxis = 1; break; // project from Z
        default: uAxis = 0; vAxis = 2; break; // project from Y (default)
    }

    std::vector<glm::vec2> uvs;
    uvs.reserve(vertices.size());

    for (const auto& v : vertices) {
        float u = normalizeRange(v[uAxis], minB[uAxis], maxB[uAxis]);
        float vCoord = normalizeRange(v[vAxis], minB[vAxis], maxB[vAxis]);
        uvs.emplace_back(u, vCoord);
    }

    mesh.setUVs(uvs);
}

// ---------------------------------------------------------------------------
// Box projection
// ---------------------------------------------------------------------------

void UVMapping::projectBox(HalfEdgeMesh& mesh) {
    const auto& vertices = mesh.getVertices();
    if (vertices.empty()) return;

    auto faces = mesh.getFaceVertexIndices();

    glm::vec3 minB, maxB;
    computeBounds(vertices, minB, maxB);

    // Initialize all UVs to zero
    std::vector<glm::vec2> uvs(vertices.size(), glm::vec2(0.0f));

    // For each face, compute its normal to determine the dominant axis,
    // then apply planar projection from that axis for its vertices
    for (const auto& face : faces) {
        if (face.size() < 3) continue;

        // Compute face normal from first three vertices
        const glm::vec3& v0 = vertices[face[0]];
        const glm::vec3& v1 = vertices[face[1]];
        const glm::vec3& v2 = vertices[face[2]];

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 normal = glm::cross(edge1, edge2);

        // Find dominant axis (largest absolute component of normal)
        glm::vec3 absNormal = glm::abs(normal);
        int dominantAxis;
        if (absNormal.x >= absNormal.y && absNormal.x >= absNormal.z) {
            dominantAxis = 0; // X dominant -> project from X: U=Z, V=Y
        } else if (absNormal.y >= absNormal.x && absNormal.y >= absNormal.z) {
            dominantAxis = 1; // Y dominant -> project from Y: U=X, V=Z
        } else {
            dominantAxis = 2; // Z dominant -> project from Z: U=X, V=Y
        }

        int uAxis, vAxis;
        switch (dominantAxis) {
            case 0: uAxis = 2; vAxis = 1; break;
            case 2: uAxis = 0; vAxis = 1; break;
            default: uAxis = 0; vAxis = 2; break;
        }

        // Apply planar projection for each vertex of this face
        for (uint32_t idx : face) {
            const glm::vec3& v = vertices[idx];
            float u = normalizeRange(v[uAxis], minB[uAxis], maxB[uAxis]);
            float vCoord = normalizeRange(v[vAxis], minB[vAxis], maxB[vAxis]);
            uvs[idx] = glm::vec2(u, vCoord);
        }
    }

    mesh.setUVs(uvs);
}

// ---------------------------------------------------------------------------
// Cylindrical projection
// ---------------------------------------------------------------------------

void UVMapping::projectCylindrical(HalfEdgeMesh& mesh) {
    const auto& vertices = mesh.getVertices();
    if (vertices.empty()) return;

    glm::vec3 minB, maxB;
    computeBounds(vertices, minB, maxB);

    // Center the projection around the mesh center in XZ
    float centerX = (minB.x + maxB.x) * 0.5f;
    float centerZ = (minB.z + maxB.z) * 0.5f;

    std::vector<glm::vec2> uvs;
    uvs.reserve(vertices.size());

    for (const auto& v : vertices) {
        // U: angle around Y axis from XZ position, mapped to [0, 1]
        float dx = v.x - centerX;
        float dz = v.z - centerZ;
        float angle = std::atan2(dz, dx); // range [-pi, pi]
        float u = (angle + glm::pi<float>()) / (2.0f * glm::pi<float>());

        // V: height along Y axis, normalized to [0, 1]
        float vCoord = normalizeRange(v.y, minB.y, maxB.y);

        uvs.emplace_back(u, vCoord);
    }

    mesh.setUVs(uvs);
}

// ---------------------------------------------------------------------------
// Spherical projection
// ---------------------------------------------------------------------------

void UVMapping::projectSpherical(HalfEdgeMesh& mesh) {
    const auto& vertices = mesh.getVertices();
    if (vertices.empty()) return;

    glm::vec3 minB, maxB;
    computeBounds(vertices, minB, maxB);

    // Center the projection around the mesh centroid
    glm::vec3 center = (minB + maxB) * 0.5f;

    std::vector<glm::vec2> uvs;
    uvs.reserve(vertices.size());

    for (const auto& v : vertices) {
        glm::vec3 d = v - center;
        float len = glm::length(d);

        float u, vCoord;
        if (len < 1e-7f) {
            // Degenerate case: vertex is at the center
            u = 0.5f;
            vCoord = 0.5f;
        } else {
            // Normalize direction
            glm::vec3 n = d / len;

            // U: longitude angle from XZ plane, mapped to [0, 1]
            u = (std::atan2(n.z, n.x) + glm::pi<float>()) / (2.0f * glm::pi<float>());

            // V: latitude from Y axis, mapped to [0, 1]
            // asin returns [-pi/2, pi/2], we remap to [0, 1]
            float clamped = glm::clamp(n.y, -1.0f, 1.0f);
            vCoord = std::asin(clamped) / glm::pi<float>() + 0.5f;
        }

        uvs.emplace_back(u, vCoord);
    }

    mesh.setUVs(uvs);
}

} // namespace PixellentModeler
