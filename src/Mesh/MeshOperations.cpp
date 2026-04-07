#include "PixellentModeler/Mesh/MeshOperations.hpp"

#include <algorithm>
#include <iostream>
#include <map>
#include <set>

namespace PixellentModeler {
namespace MeshOperations {

// ---------------------------------------------------------------------------
// extrudeFaces
// ---------------------------------------------------------------------------

ExtrudeResult extrudeFaces(HalfEdgeMesh& mesh, const std::vector<uint32_t>& faceIds, float distance) {
    ExtrudeResult result;

    if (faceIds.empty()) return result;

    // Collect the set of selected faces for quick lookup
    std::set<uint32_t> selectedFaces(faceIds.begin(), faceIds.end());

    // For each selected face, collect its vertices and the face normal
    // We need to:
    // 1. Duplicate the vertices of each selected face, offset by normal * distance
    // 2. Reconnect the face to use the new (top) vertices
    // 3. Create side quad faces connecting old verts to new verts

    // First pass: collect all unique vertices involved in the extrusion
    // and track which edges are "boundary" edges of the selection region
    // (i.e., edges that separate a selected face from an unselected one or from nothing)
    std::map<uint32_t, uint32_t> oldToNewVertex; // maps old vertex index -> new (duplicated) vertex index

    // Collect all vertices from selected faces
    for (uint32_t faceId : faceIds) {
        if (faceId >= mesh.faceCount()) continue;

        auto verts = mesh.faceVertices(faceId);
        for (uint32_t v : verts) {
            if (oldToNewVertex.find(v) == oldToNewVertex.end()) {
                oldToNewVertex[v] = INVALID_INDEX; // placeholder
            }
        }
    }

    // Create duplicate vertices offset by the face normal * distance
    // For shared vertices between multiple selected faces, we average the normals
    // First compute the offset direction per vertex
    std::map<uint32_t, glm::vec3> vertexOffsets;
    std::map<uint32_t, int> vertexFaceCount;

    for (uint32_t faceId : faceIds) {
        if (faceId >= mesh.faceCount()) continue;

        glm::vec3 faceNormal = mesh.faces[faceId].normal;
        auto verts = mesh.faceVertices(faceId);
        for (uint32_t v : verts) {
            vertexOffsets[v] += faceNormal;
            vertexFaceCount[v]++;
        }
    }

    // Normalize the offset direction for each vertex and create new vertices
    for (auto& [oldVert, newVert] : oldToNewVertex) {
        glm::vec3 offset = vertexOffsets[oldVert];
        int count = vertexFaceCount[oldVert];
        if (count > 0) {
            offset /= static_cast<float>(count);
        }
        float len = glm::length(offset);
        if (len > 1e-8f) {
            offset = (offset / len) * distance;
        } else {
            offset = glm::vec3(0.0f, distance, 0.0f);
        }

        glm::vec3 newPos = mesh.vertices[oldVert].position + offset;
        newVert = mesh.addVertex(newPos);
        mesh.vertices[newVert].uv = mesh.vertices[oldVert].uv;
        result.newVertices.push_back(newVert);
    }

    // Now we need to rebuild the mesh topology.
    // Strategy: collect all original data, modify it, and rebuild.

    // Collect all current mesh data into polygon representation
    std::vector<glm::vec3> allPositions;
    allPositions.reserve(mesh.vertices.size());
    for (const auto& v : mesh.vertices) {
        allPositions.push_back(v.position);
    }

    // Collect UVs before rebuild
    std::vector<glm::vec2> allUVs;
    allUVs.reserve(mesh.vertices.size());
    for (const auto& v : mesh.vertices) {
        allUVs.push_back(v.uv);
    }

    std::vector<std::vector<uint32_t>> allFaces;
    allFaces.reserve(mesh.faceCount());

    for (uint32_t fi = 0; fi < mesh.faceCount(); ++fi) {
        allFaces.push_back(mesh.faceVertices(fi));
    }

    // Modify selected faces to use new vertices (the extruded top)
    for (uint32_t faceId : faceIds) {
        if (faceId >= allFaces.size()) continue;
        auto& faceVerts = allFaces[faceId];
        for (auto& v : faceVerts) {
            auto it = oldToNewVertex.find(v);
            if (it != oldToNewVertex.end()) {
                v = it->second;
            }
        }
    }

    // Find boundary edges of the selection region and create side quads.
    // A boundary edge is one where the selected face is on one side and
    // either no face or an unselected face is on the other side.
    // We need the original face vertex lists for this.
    // Go through each selected face and check each edge.
    std::set<std::pair<uint32_t, uint32_t>> processedEdges;

    for (uint32_t faceId : faceIds) {
        auto origFaceVerts = mesh.faceVertices(faceId);
        uint32_t n = static_cast<uint32_t>(origFaceVerts.size());

        for (uint32_t i = 0; i < n; ++i) {
            uint32_t v0 = origFaceVerts[i];
            uint32_t v1 = origFaceVerts[(i + 1) % n];

            // Canonical edge key
            auto edgeKey = std::make_pair(std::min(v0, v1), std::max(v0, v1));
            if (processedEdges.count(edgeKey)) continue;

            // Check if the other face sharing this edge is also selected
            bool otherFaceSelected = false;

            // Find the edge in the original mesh and check its faces
            for (uint32_t ei = 0; ei < mesh.edgeCount(); ++ei) {
                uint32_t ev0 = mesh.edgeVertex0(ei);
                uint32_t ev1 = mesh.edgeVertex1(ei);
                auto ek = std::make_pair(std::min(ev0, ev1), std::max(ev0, ev1));
                if (ek == edgeKey) {
                    auto adjFaces = mesh.edgeFaces(ei);
                    for (uint32_t af : adjFaces) {
                        if (af != faceId && selectedFaces.count(af)) {
                            otherFaceSelected = true;
                        }
                    }
                    break;
                }
            }

            // Only create a side quad for boundary edges of the selection
            if (!otherFaceSelected) {
                processedEdges.insert(edgeKey);

                uint32_t newV0 = oldToNewVertex[v0];
                uint32_t newV1 = oldToNewVertex[v1];

                // Side quad: old_v0 -> old_v1 -> new_v1 -> new_v0 (CCW from outside)
                // The winding depends on which side the selected face is on.
                // Since the face has v0->v1 in CCW order (from outside), the side
                // quad should go: v1 -> v0 -> newV0 -> newV1 (CCW from outside)
                allFaces.push_back({v1, v0, newV0, newV1});
            }
        }
    }

    // Track the new face indices (the modified selected faces + the new side quads)
    // The modified selected faces keep their original indices.
    // The side quads are appended at the end.
    for (uint32_t faceId : faceIds) {
        result.newFaces.push_back(faceId);
    }
    uint32_t sideQuadStart = static_cast<uint32_t>(allFaces.size()) -
                             static_cast<uint32_t>(processedEdges.size());
    for (uint32_t i = sideQuadStart; i < static_cast<uint32_t>(allFaces.size()); ++i) {
        result.newFaces.push_back(i);
    }

    // Rebuild the mesh from the modified polygon data
    mesh.buildFromPolygons(allPositions, allFaces);

    // Restore UVs
    for (uint32_t i = 0; i < static_cast<uint32_t>(allUVs.size()) && i < mesh.vertexCount(); ++i) {
        mesh.vertices[i].uv = allUVs[i];
    }

    return result;
}

// ---------------------------------------------------------------------------
// deleteFaces
// ---------------------------------------------------------------------------

void deleteFaces(HalfEdgeMesh& mesh, const std::vector<uint32_t>& faceIds) {
    if (faceIds.empty()) return;

    // Collect the set of faces to delete
    std::set<uint32_t> toDelete(faceIds.begin(), faceIds.end());

    // Collect all data for rebuild, excluding deleted faces
    std::vector<glm::vec3> positions;
    positions.reserve(mesh.vertexCount());
    for (const auto& v : mesh.vertices) {
        positions.push_back(v.position);
    }

    std::vector<glm::vec2> uvs;
    uvs.reserve(mesh.vertexCount());
    for (const auto& v : mesh.vertices) {
        uvs.push_back(v.uv);
    }

    std::vector<std::vector<uint32_t>> remainingFaces;
    for (uint32_t fi = 0; fi < mesh.faceCount(); ++fi) {
        if (toDelete.count(fi)) continue;
        remainingFaces.push_back(mesh.faceVertices(fi));
    }

    // Determine which vertices are still used by remaining faces
    std::set<uint32_t> usedVertices;
    for (const auto& face : remainingFaces) {
        for (uint32_t v : face) {
            usedVertices.insert(v);
        }
    }

    // Create a vertex remapping to compact the vertex array
    std::map<uint32_t, uint32_t> vertexRemap;
    std::vector<glm::vec3> newPositions;
    std::vector<glm::vec2> newUVs;

    for (uint32_t oldIdx : usedVertices) {
        uint32_t newIdx = static_cast<uint32_t>(newPositions.size());
        vertexRemap[oldIdx] = newIdx;
        newPositions.push_back(positions[oldIdx]);
        if (oldIdx < uvs.size()) {
            newUVs.push_back(uvs[oldIdx]);
        } else {
            newUVs.push_back({0.0f, 0.0f});
        }
    }

    // Remap face vertex indices
    for (auto& face : remainingFaces) {
        for (auto& v : face) {
            v = vertexRemap[v];
        }
    }

    // Rebuild the mesh
    mesh.buildFromPolygons(newPositions, remainingFaces);

    // Restore UVs
    for (uint32_t i = 0; i < static_cast<uint32_t>(newUVs.size()) && i < mesh.vertexCount(); ++i) {
        mesh.vertices[i].uv = newUVs[i];
    }
}

// ---------------------------------------------------------------------------
// Stub implementations
// ---------------------------------------------------------------------------

void insetFaces(HalfEdgeMesh& /*mesh*/, const std::vector<uint32_t>& /*faceIds*/, float /*amount*/) {
    std::cout << "MeshOperations::insetFaces: not yet implemented" << std::endl;
}

void bevelEdges(HalfEdgeMesh& /*mesh*/, const std::vector<uint32_t>& /*edgeIds*/, float /*width*/) {
    std::cout << "MeshOperations::bevelEdges: not yet implemented" << std::endl;
}

void subdivide(HalfEdgeMesh& /*mesh*/) {
    std::cout << "MeshOperations::subdivide (Catmull-Clark): not yet implemented" << std::endl;
}

void loopCut(HalfEdgeMesh& /*mesh*/, uint32_t /*edgeId*/, int /*cuts*/) {
    std::cout << "MeshOperations::loopCut: not yet implemented" << std::endl;
}

void mergeVertices(HalfEdgeMesh& /*mesh*/, const std::vector<uint32_t>& /*vertexIds*/) {
    std::cout << "MeshOperations::mergeVertices: not yet implemented" << std::endl;
}

void deleteEdges(HalfEdgeMesh& /*mesh*/, const std::vector<uint32_t>& /*edgeIds*/) {
    std::cout << "MeshOperations::deleteEdges: not yet implemented" << std::endl;
}

void deleteVertices(HalfEdgeMesh& /*mesh*/, const std::vector<uint32_t>& /*vertexIds*/) {
    std::cout << "MeshOperations::deleteVertices: not yet implemented" << std::endl;
}

} // namespace MeshOperations
} // namespace PixellentModeler
