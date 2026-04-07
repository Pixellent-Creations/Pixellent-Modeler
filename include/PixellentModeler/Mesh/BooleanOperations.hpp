#pragma once

#include "PixellentModeler/Mesh/HalfEdgeMesh.hpp"
#include <vector>

namespace PixellentModeler {

namespace BooleanOperations {

enum class BoolOpType {
    Union,
    Difference,
    Intersection
};

// Simplified boolean operation wrapper
// In production, would use carve or similar CSG library via FetchContent
HalfEdgeMesh performBoolean(const HalfEdgeMesh& meshA,
                            const HalfEdgeMesh& meshB,
                            BoolOpType operation);

// Individual operations
HalfEdgeMesh unionMeshes(const HalfEdgeMesh& meshA, const HalfEdgeMesh& meshB);
HalfEdgeMesh differenceMeshes(const HalfEdgeMesh& meshA, const HalfEdgeMesh& meshB);
HalfEdgeMesh intersectionMeshes(const HalfEdgeMesh& meshA, const HalfEdgeMesh& meshB);

} // namespace BooleanOperations
} // namespace PixellentModeler
