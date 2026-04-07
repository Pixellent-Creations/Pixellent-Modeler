#include "PixellentModeler/Scene/ModifierComponent.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace PixellentModeler {

// ============================================
// ArrayModifier
// ============================================

HalfEdgeMesh ArrayModifier::apply(const HalfEdgeMesh& baseMesh) const {
    HalfEdgeMesh result = baseMesh;

    // For each X, Y, Z count, duplicate and offset the mesh
    for (int x = 0; x < countX; ++x) {
        for (int y = 0; y < countY; ++y) {
            for (int z = 0; z < countZ; ++z) {
                if (x == 0 && y == 0 && z == 0) continue; // Skip original

                // Calculate offset
                glm::vec3 offset(x * offsetX, y * offsetY, z * offsetZ);

                // Add duplicated vertices with offset
                for (uint32_t i = 0; i < baseMesh.vertexCount(); ++i) {
                    glm::vec3 pos = baseMesh.vertices[i].position + offset;
                    result.addVertex(pos);
                }
            }
        }
    }

    result.recomputeNormals();
    return result;
}

// ============================================
// MirrorModifier
// ============================================

HalfEdgeMesh MirrorModifier::apply(const HalfEdgeMesh& baseMesh) const {
    HalfEdgeMesh result = baseMesh;

    // Mirror across the specified axis
    for (uint32_t i = 0; i < baseMesh.vertexCount(); ++i) {
        glm::vec3 pos = baseMesh.vertices[i].position;

        // Mirror position
        pos[axis] = -pos[axis] + offset;

        result.addVertex(pos);
    }

    // Flip face winding when mirrored
    result.recomputeNormals();
    return result;
}

// ============================================
// BevelModifier
// ============================================

HalfEdgeMesh BevelModifier::apply(const HalfEdgeMesh& baseMesh) const {
    HalfEdgeMesh result = baseMesh;

    if (edgeBevel) {
        // For each edge, create beveled geometry
        for (uint32_t i = 0; i < baseMesh.edgeCount(); ++i) {
            uint32_t v0 = baseMesh.edgeVertex0(i);
            uint32_t v1 = baseMesh.edgeVertex1(i);

            if (v0 >= baseMesh.vertexCount() || v1 >= baseMesh.vertexCount()) continue;

            glm::vec3 p0 = baseMesh.vertices[v0].position;
            glm::vec3 p1 = baseMesh.vertices[v1].position;

            // Create vertices along the edge
            glm::vec3 d = glm::normalize(p1 - p0) * amount;
            result.addVertex(p0 + d);
            result.addVertex(p1 - d);
        }
    }

    if (vertexBevel) {
        // For each vertex, create beveled edges
        for (uint32_t i = 0; i < baseMesh.vertexCount(); ++i) {
            auto neighbors = baseMesh.vertexNeighbors(i);
            for (uint32_t n : neighbors) {
                glm::vec3 dir = glm::normalize(baseMesh.vertices[n].position - baseMesh.vertices[i].position);
                result.addVertex(baseMesh.vertices[i].position + dir * amount);
            }
        }
    }

    result.recomputeNormals();
    return result;
}

} // namespace PixellentModeler
