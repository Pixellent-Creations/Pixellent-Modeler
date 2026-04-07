#pragma once

#include <vector>
#include <cstdint>

#include "PixellentModeler/Mesh/HalfEdgeMesh.hpp"

namespace PixellentModeler {
namespace MeshOperations {

struct ExtrudeResult {
    std::vector<uint32_t> newFaces;
    std::vector<uint32_t> newVertices;
};

ExtrudeResult extrudeFaces(HalfEdgeMesh& mesh, const std::vector<uint32_t>& faceIds, float distance);
void insetFaces(HalfEdgeMesh& mesh, const std::vector<uint32_t>& faceIds, float amount);
void bevelEdges(HalfEdgeMesh& mesh, const std::vector<uint32_t>& edgeIds, float width);
void subdivide(HalfEdgeMesh& mesh);  // Catmull-Clark subdivision
void loopCut(HalfEdgeMesh& mesh, uint32_t edgeId, int cuts = 1);
void mergeVertices(HalfEdgeMesh& mesh, const std::vector<uint32_t>& vertexIds);
void deleteFaces(HalfEdgeMesh& mesh, const std::vector<uint32_t>& faceIds);
void deleteEdges(HalfEdgeMesh& mesh, const std::vector<uint32_t>& edgeIds);
void deleteVertices(HalfEdgeMesh& mesh, const std::vector<uint32_t>& vertexIds);

} // namespace MeshOperations
} // namespace PixellentModeler
