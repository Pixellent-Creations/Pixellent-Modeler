#include "PixellentModeler/Mesh/HalfEdgeMesh.hpp"

#include <map>
#include <set>
#include <iostream>
#include <algorithm>
#include <cmath>

namespace PixellentModeler {

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

uint32_t HalfEdgeMesh::addVertex(const glm::vec3& pos) {
    HEVertex v;
    v.position = pos;
    vertices.push_back(v);
    return static_cast<uint32_t>(vertices.size() - 1);
}

uint32_t HalfEdgeMesh::addFace(const std::vector<uint32_t>& vertexIndices) {
    if (vertexIndices.size() < 3) {
        std::cerr << "HalfEdgeMesh::addFace: need at least 3 vertices" << std::endl;
        return INVALID_INDEX;
    }

    uint32_t faceIdx = static_cast<uint32_t>(faces.size());
    HEFace face;
    faces.push_back(face);

    uint32_t n = static_cast<uint32_t>(vertexIndices.size());
    uint32_t firstHE = static_cast<uint32_t>(halfEdges.size());

    // Create half-edges for this face
    for (uint32_t i = 0; i < n; ++i) {
        HEHalfEdge he;
        he.vertex = vertexIndices[(i + 1) % n]; // points to the next vertex
        he.face = faceIdx;
        halfEdges.push_back(he);
    }

    // Link next/prev within the face loop
    for (uint32_t i = 0; i < n; ++i) {
        uint32_t heIdx = firstHE + i;
        halfEdges[heIdx].next = firstHE + ((i + 1) % n);
        halfEdges[heIdx].prev = firstHE + ((i + n - 1) % n);
    }

    // Point face to its first half-edge
    faces[faceIdx].halfEdge = firstHE;

    // Assign outgoing half-edge to each vertex (if not already set)
    for (uint32_t i = 0; i < n; ++i) {
        uint32_t vIdx = vertexIndices[i];
        uint32_t heIdx = firstHE + i; // this half-edge goes FROM vertexIndices[i]
        if (vertices[vIdx].halfEdge == INVALID_INDEX) {
            vertices[vIdx].halfEdge = heIdx;
        }
    }

    // Try to pair twins with existing half-edges using a brute-force search
    // This is O(n * halfEdgeCount) but is only used for incremental addFace calls.
    // For bulk building, use buildFromPolygons which uses a map.
    for (uint32_t i = 0; i < n; ++i) {
        uint32_t heIdx = firstHE + i;
        uint32_t v0 = vertexIndices[i];
        uint32_t v1 = vertexIndices[(i + 1) % n];

        if (halfEdges[heIdx].twin != INVALID_INDEX)
            continue;

        // Search for a half-edge going from v1 to v0
        for (uint32_t j = 0; j < firstHE; ++j) {
            if (halfEdges[j].twin != INVALID_INDEX)
                continue;
            // Half-edge j goes from some vertex to halfEdges[j].vertex
            // We need to figure out which vertex j originates from.
            // The origin of half-edge j is halfEdges[halfEdges[j].prev].vertex
            uint32_t prevJ = halfEdges[j].prev;
            if (prevJ == INVALID_INDEX)
                continue;
            uint32_t originJ = halfEdges[prevJ].vertex;
            uint32_t destJ = halfEdges[j].vertex;

            if (originJ == v1 && destJ == v0) {
                halfEdges[heIdx].twin = j;
                halfEdges[j].twin = heIdx;

                // Create an edge entry for this pair
                HEEdge edge;
                edge.halfEdge = heIdx;
                edges.push_back(edge);
                break;
            }
        }

        // If no twin found, this is a boundary edge -- create an edge entry anyway
        if (halfEdges[heIdx].twin == INVALID_INDEX) {
            HEEdge edge;
            edge.halfEdge = heIdx;
            edges.push_back(edge);
        }
    }

    return faceIdx;
}

// ---------------------------------------------------------------------------
// Bulk construction
// ---------------------------------------------------------------------------

void HalfEdgeMesh::buildFromPolygons(const std::vector<glm::vec3>& positions,
                                     const std::vector<std::vector<uint32_t>>& faceIndices) {
    clear();

    // Add all vertices
    vertices.reserve(positions.size());
    for (const auto& pos : positions) {
        addVertex(pos);
    }

    // Map from directed edge (v0, v1) -> half-edge index
    std::map<std::pair<uint32_t, uint32_t>, uint32_t> edgeMap;

    // Create all faces and their half-edges
    for (const auto& faceVerts : faceIndices) {
        if (faceVerts.size() < 3) continue;

        uint32_t faceIdx = static_cast<uint32_t>(faces.size());
        HEFace face;
        faces.push_back(face);

        uint32_t n = static_cast<uint32_t>(faceVerts.size());
        uint32_t firstHE = static_cast<uint32_t>(halfEdges.size());

        // Create half-edges for this face
        for (uint32_t i = 0; i < n; ++i) {
            HEHalfEdge he;
            he.vertex = faceVerts[(i + 1) % n]; // points to next vertex in face
            he.face = faceIdx;
            halfEdges.push_back(he);
        }

        // Link next/prev within the face loop
        for (uint32_t i = 0; i < n; ++i) {
            uint32_t heIdx = firstHE + i;
            halfEdges[heIdx].next = firstHE + ((i + 1) % n);
            halfEdges[heIdx].prev = firstHE + ((i + n - 1) % n);
        }

        // Point face to its first half-edge
        faces[faceIdx].halfEdge = firstHE;

        // Set vertex outgoing half-edges and register edges in map
        for (uint32_t i = 0; i < n; ++i) {
            uint32_t vIdx = faceVerts[i];
            uint32_t heIdx = firstHE + i;

            // Set vertex outgoing half-edge if not already set
            if (vertices[vIdx].halfEdge == INVALID_INDEX) {
                vertices[vIdx].halfEdge = heIdx;
            }

            // Register this directed edge in the map for twin pairing
            uint32_t v0 = faceVerts[i];
            uint32_t v1 = faceVerts[(i + 1) % n];
            edgeMap[{v0, v1}] = heIdx;
        }
    }

    // Pair twins and create edge entries
    std::set<std::pair<uint32_t, uint32_t>> processedEdges;

    for (auto& [directedEdge, heIdx] : edgeMap) {
        uint32_t v0 = directedEdge.first;
        uint32_t v1 = directedEdge.second;

        // Canonical edge key: smaller vertex first
        auto canonicalKey = std::make_pair(std::min(v0, v1), std::max(v0, v1));

        // Look for twin: the half-edge going from v1 to v0
        auto twinIt = edgeMap.find({v1, v0});
        if (twinIt != edgeMap.end()) {
            uint32_t twinIdx = twinIt->second;
            halfEdges[heIdx].twin = twinIdx;
            halfEdges[twinIdx].twin = heIdx;
        }

        // Create a single edge entry for this undirected edge
        if (processedEdges.find(canonicalKey) == processedEdges.end()) {
            processedEdges.insert(canonicalKey);
            HEEdge edge;
            edge.halfEdge = heIdx;
            edges.push_back(edge);
        }
    }

    recomputeNormals();
}

void HalfEdgeMesh::buildFromTriangles(const std::vector<glm::vec3>& positions,
                                      const std::vector<uint32_t>& indices) {
    std::vector<std::vector<uint32_t>> faceIndices;
    faceIndices.reserve(indices.size() / 3);

    for (size_t i = 0; i + 2 < indices.size(); i += 3) {
        faceIndices.push_back({indices[i], indices[i + 1], indices[i + 2]});
    }

    buildFromPolygons(positions, faceIndices);
}

// ---------------------------------------------------------------------------
// Topology queries
// ---------------------------------------------------------------------------

std::vector<uint32_t> HalfEdgeMesh::vertexFaces(uint32_t vertId) const {
    std::vector<uint32_t> result;
    if (vertId >= vertices.size()) return result;

    uint32_t startHE = vertices[vertId].halfEdge;
    if (startHE == INVALID_INDEX) return result;

    uint32_t he = startHE;
    do {
        if (halfEdges[he].face != INVALID_INDEX) {
            result.push_back(halfEdges[he].face);
        }
        // Move to the next half-edge around this vertex:
        // go to the twin of the current half-edge, then follow next
        uint32_t twin = halfEdges[he].twin;
        if (twin == INVALID_INDEX) break;
        he = halfEdges[twin].next;
        if (he == INVALID_INDEX) break;
    } while (he != startHE);

    // If we broke out early (boundary vertex), also traverse the other direction
    if (he != startHE) {
        // Go backwards from the start half-edge
        he = startHE;
        while (true) {
            uint32_t prevHE = halfEdges[he].prev;
            if (prevHE == INVALID_INDEX) break;
            uint32_t twin = halfEdges[prevHE].twin;
            if (twin == INVALID_INDEX) break;
            he = twin;
            if (he == startHE) break;
            if (halfEdges[he].face != INVALID_INDEX) {
                result.push_back(halfEdges[he].face);
            }
        }
    }

    return result;
}

std::vector<uint32_t> HalfEdgeMesh::vertexNeighbors(uint32_t vertId) const {
    std::vector<uint32_t> result;
    if (vertId >= vertices.size()) return result;

    uint32_t startHE = vertices[vertId].halfEdge;
    if (startHE == INVALID_INDEX) return result;

    uint32_t he = startHE;
    do {
        // The half-edge 'he' originates from vertId and points to halfEdges[he].vertex
        result.push_back(halfEdges[he].vertex);

        uint32_t twin = halfEdges[he].twin;
        if (twin == INVALID_INDEX) break;
        he = halfEdges[twin].next;
        if (he == INVALID_INDEX) break;
    } while (he != startHE);

    // Handle boundary: traverse backward
    if (he != startHE) {
        he = startHE;
        while (true) {
            uint32_t prevHE = halfEdges[he].prev;
            if (prevHE == INVALID_INDEX) break;
            uint32_t twin = halfEdges[prevHE].twin;
            if (twin == INVALID_INDEX) {
                // The prev half-edge's origin is also a neighbor
                // prevHE points to vertId (since it's the previous in the face loop)
                // prevHE originates from halfEdges[halfEdges[prevHE].prev].vertex
                // Actually, prev half-edge points to the start of our half-edge,
                // and the vertex it comes from is what we want
                result.push_back(halfEdges[prevHE].vertex);
                break;
            }
            he = twin;
            if (he == startHE) break;
            result.push_back(halfEdges[he].vertex);
        }
    }

    return result;
}

std::vector<uint32_t> HalfEdgeMesh::vertexEdges(uint32_t vertId) const {
    std::vector<uint32_t> result;
    if (vertId >= vertices.size()) return result;

    // Find all edges that touch this vertex by checking each edge
    for (uint32_t i = 0; i < static_cast<uint32_t>(edges.size()); ++i) {
        if (edgeVertex0(i) == vertId || edgeVertex1(i) == vertId) {
            result.push_back(i);
        }
    }

    return result;
}

std::vector<uint32_t> HalfEdgeMesh::faceVertices(uint32_t faceId) const {
    std::vector<uint32_t> result;
    if (faceId >= faces.size()) return result;

    uint32_t startHE = faces[faceId].halfEdge;
    if (startHE == INVALID_INDEX) return result;

    uint32_t he = startHE;
    do {
        // The half-edge originates from some vertex. We find its origin.
        // Origin of he = halfEdges[halfEdges[he].prev].vertex
        // But it's simpler to collect the vertex each half-edge points to:
        // that gives us all face vertices in CCW order.
        result.push_back(halfEdges[he].vertex);
        he = halfEdges[he].next;
    } while (he != startHE && he != INVALID_INDEX);

    return result;
}

std::vector<uint32_t> HalfEdgeMesh::faceEdges(uint32_t faceId) const {
    std::vector<uint32_t> result;
    if (faceId >= faces.size()) return result;

    uint32_t startHE = faces[faceId].halfEdge;
    if (startHE == INVALID_INDEX) return result;

    uint32_t he = startHE;
    do {
        // Find the edge that this half-edge belongs to
        for (uint32_t i = 0; i < static_cast<uint32_t>(edges.size()); ++i) {
            uint32_t eHE = edges[i].halfEdge;
            if (eHE == he || (halfEdges[eHE].twin == he)) {
                result.push_back(i);
                break;
            }
        }
        he = halfEdges[he].next;
    } while (he != startHE && he != INVALID_INDEX);

    return result;
}

std::vector<uint32_t> HalfEdgeMesh::edgeFaces(uint32_t edgeId) const {
    std::vector<uint32_t> result;
    if (edgeId >= edges.size()) return result;

    uint32_t he = edges[edgeId].halfEdge;
    if (he == INVALID_INDEX) return result;

    if (halfEdges[he].face != INVALID_INDEX)
        result.push_back(halfEdges[he].face);

    uint32_t twin = halfEdges[he].twin;
    if (twin != INVALID_INDEX && halfEdges[twin].face != INVALID_INDEX)
        result.push_back(halfEdges[twin].face);

    return result;
}

uint32_t HalfEdgeMesh::edgeVertex0(uint32_t edgeId) const {
    if (edgeId >= edges.size()) return INVALID_INDEX;

    uint32_t he = edges[edgeId].halfEdge;
    if (he == INVALID_INDEX) return INVALID_INDEX;

    // Origin of the half-edge: the vertex that the previous half-edge points to
    uint32_t prevHE = halfEdges[he].prev;
    if (prevHE == INVALID_INDEX) return INVALID_INDEX;

    return halfEdges[prevHE].vertex;
}

uint32_t HalfEdgeMesh::edgeVertex1(uint32_t edgeId) const {
    if (edgeId >= edges.size()) return INVALID_INDEX;

    uint32_t he = edges[edgeId].halfEdge;
    if (he == INVALID_INDEX) return INVALID_INDEX;

    return halfEdges[he].vertex;
}

bool HalfEdgeMesh::isBoundaryVertex(uint32_t vertId) const {
    if (vertId >= vertices.size()) return false;

    uint32_t startHE = vertices[vertId].halfEdge;
    if (startHE == INVALID_INDEX) return true; // isolated vertex

    uint32_t he = startHE;
    do {
        if (halfEdges[he].twin == INVALID_INDEX)
            return true;
        he = halfEdges[halfEdges[he].twin].next;
        if (he == INVALID_INDEX) return true;
    } while (he != startHE);

    return false;
}

bool HalfEdgeMesh::isBoundaryEdge(uint32_t edgeId) const {
    if (edgeId >= edges.size()) return false;

    uint32_t he = edges[edgeId].halfEdge;
    if (he == INVALID_INDEX) return true;

    return halfEdges[he].twin == INVALID_INDEX;
}

// ---------------------------------------------------------------------------
// Geometry
// ---------------------------------------------------------------------------

void HalfEdgeMesh::recomputeNormals() {
    // Compute face normals
    for (uint32_t fi = 0; fi < static_cast<uint32_t>(faces.size()); ++fi) {
        auto verts = faceVertices(fi);
        if (verts.size() < 3) {
            faces[fi].normal = glm::vec3(0, 1, 0);
            continue;
        }

        // Use Newell's method for robust normal computation on polygons
        glm::vec3 normal(0.0f);
        for (size_t i = 0; i < verts.size(); ++i) {
            const glm::vec3& current = vertices[verts[i]].position;
            const glm::vec3& next = vertices[verts[(i + 1) % verts.size()]].position;
            normal.x += (current.y - next.y) * (current.z + next.z);
            normal.y += (current.z - next.z) * (current.x + next.x);
            normal.z += (current.x - next.x) * (current.y + next.y);
        }

        float len = glm::length(normal);
        if (len > 1e-8f) {
            faces[fi].normal = normal / len;
        } else {
            faces[fi].normal = glm::vec3(0, 1, 0);
        }
    }

    // Compute vertex normals as average of adjacent face normals
    for (auto& v : vertices) {
        v.normal = glm::vec3(0.0f);
    }

    for (uint32_t fi = 0; fi < static_cast<uint32_t>(faces.size()); ++fi) {
        auto verts = faceVertices(fi);
        for (uint32_t vi : verts) {
            vertices[vi].normal += faces[fi].normal;
        }
    }

    for (auto& v : vertices) {
        float len = glm::length(v.normal);
        if (len > 1e-8f) {
            v.normal /= len;
        } else {
            v.normal = glm::vec3(0, 1, 0);
        }
    }
}

BoundingBox HalfEdgeMesh::computeBounds() const {
    BoundingBox bb;
    for (const auto& v : vertices) {
        bb.expand(v.position);
    }
    return bb;
}

// ---------------------------------------------------------------------------
// Triangulation
// ---------------------------------------------------------------------------

TriMeshData HalfEdgeMesh::triangulate() const {
    TriMeshData result;

    // First, collect unique vertices and their data
    // For each face, fan-triangulate using the first vertex as the hub
    uint32_t vertexIndex = 0;

    for (uint32_t fi = 0; fi < static_cast<uint32_t>(faces.size()); ++fi) {
        auto verts = faceVertices(fi);
        if (verts.size() < 3) continue;

        uint32_t firstVert = vertexIndex;

        // Output all vertices for this face
        for (uint32_t vi : verts) {
            const HEVertex& v = vertices[vi];
            result.vertices.push_back(v.position.x);
            result.vertices.push_back(v.position.y);
            result.vertices.push_back(v.position.z);
            result.vertices.push_back(v.normal.x);
            result.vertices.push_back(v.normal.y);
            result.vertices.push_back(v.normal.z);
            result.vertices.push_back(v.uv.x);
            result.vertices.push_back(v.uv.y);
            ++vertexIndex;
        }

        // Fan triangulation: vertex 0, vertex i, vertex i+1
        for (uint32_t i = 1; i + 1 < static_cast<uint32_t>(verts.size()); ++i) {
            result.indices.push_back(firstVert);
            result.indices.push_back(firstVert + i);
            result.indices.push_back(firstVert + i + 1);
        }
    }

    return result;
}

// ---------------------------------------------------------------------------
// Wireframe
// ---------------------------------------------------------------------------

HalfEdgeMesh::WireframeData HalfEdgeMesh::wireframeData() const {
    WireframeData result;

    // Output all vertex positions
    result.vertices.reserve(vertices.size() * 3);
    for (const auto& v : vertices) {
        result.vertices.push_back(v.position.x);
        result.vertices.push_back(v.position.y);
        result.vertices.push_back(v.position.z);
    }

    // Output line indices for each edge
    result.indices.reserve(edges.size() * 2);
    for (uint32_t i = 0; i < static_cast<uint32_t>(edges.size()); ++i) {
        uint32_t v0 = edgeVertex0(i);
        uint32_t v1 = edgeVertex1(i);
        if (v0 != INVALID_INDEX && v1 != INVALID_INDEX) {
            result.indices.push_back(v0);
            result.indices.push_back(v1);
        }
    }

    return result;
}

// ---------------------------------------------------------------------------
// Validation
// ---------------------------------------------------------------------------

bool HalfEdgeMesh::validate() const {
    bool valid = true;

    // Check twin reciprocity
    for (uint32_t i = 0; i < static_cast<uint32_t>(halfEdges.size()); ++i) {
        uint32_t twin = halfEdges[i].twin;
        if (twin != INVALID_INDEX) {
            if (twin >= halfEdges.size()) {
                std::cerr << "Validation: half-edge " << i
                          << " has out-of-range twin " << twin << std::endl;
                valid = false;
                continue;
            }
            if (halfEdges[twin].twin != i) {
                std::cerr << "Validation: half-edge " << i
                          << " twin is " << twin
                          << " but twin's twin is " << halfEdges[twin].twin << std::endl;
                valid = false;
            }
        }
    }

    // Check that next chains form closed loops for each face
    for (uint32_t fi = 0; fi < static_cast<uint32_t>(faces.size()); ++fi) {
        uint32_t startHE = faces[fi].halfEdge;
        if (startHE == INVALID_INDEX) {
            std::cerr << "Validation: face " << fi << " has no half-edge" << std::endl;
            valid = false;
            continue;
        }

        uint32_t he = startHE;
        uint32_t count = 0;
        const uint32_t maxIter = static_cast<uint32_t>(halfEdges.size()) + 1;
        do {
            if (halfEdges[he].face != fi) {
                std::cerr << "Validation: half-edge " << he
                          << " in face " << fi << "'s loop has face = "
                          << halfEdges[he].face << std::endl;
                valid = false;
                break;
            }
            he = halfEdges[he].next;
            ++count;
            if (count > maxIter) {
                std::cerr << "Validation: face " << fi
                          << " has non-closing half-edge loop" << std::endl;
                valid = false;
                break;
            }
        } while (he != startHE);
    }

    // Check prev/next consistency
    for (uint32_t i = 0; i < static_cast<uint32_t>(halfEdges.size()); ++i) {
        uint32_t nextHE = halfEdges[i].next;
        if (nextHE != INVALID_INDEX && nextHE < halfEdges.size()) {
            if (halfEdges[nextHE].prev != i) {
                std::cerr << "Validation: half-edge " << i
                          << " next is " << nextHE
                          << " but next's prev is " << halfEdges[nextHE].prev << std::endl;
                valid = false;
            }
        }
    }

    // Check vertex half-edge references
    for (uint32_t i = 0; i < static_cast<uint32_t>(vertices.size()); ++i) {
        uint32_t he = vertices[i].halfEdge;
        if (he != INVALID_INDEX) {
            if (he >= halfEdges.size()) {
                std::cerr << "Validation: vertex " << i
                          << " has out-of-range half-edge " << he << std::endl;
                valid = false;
            }
        }
    }

    // Check edge half-edge references
    for (uint32_t i = 0; i < static_cast<uint32_t>(edges.size()); ++i) {
        uint32_t he = edges[i].halfEdge;
        if (he == INVALID_INDEX || he >= halfEdges.size()) {
            std::cerr << "Validation: edge " << i
                      << " has invalid half-edge " << he << std::endl;
            valid = false;
        }
    }

    if (valid) {
        std::cout << "HalfEdgeMesh validation passed: "
                  << vertices.size() << " vertices, "
                  << edges.size() << " edges, "
                  << faces.size() << " faces, "
                  << halfEdges.size() << " half-edges" << std::endl;
    }

    return valid;
}

// ---------------------------------------------------------------------------
// Clear
// ---------------------------------------------------------------------------

void HalfEdgeMesh::clear() {
    vertices.clear();
    halfEdges.clear();
    edges.clear();
    faces.clear();
}

// ---------------------------------------------------------------------------
// Convenience accessors for serialization / IO
// ---------------------------------------------------------------------------

std::vector<glm::vec3> HalfEdgeMesh::getVertices() const {
    std::vector<glm::vec3> result;
    result.reserve(vertices.size());
    for (const auto& v : vertices) {
        result.push_back(v.position);
    }
    return result;
}

std::vector<glm::vec3> HalfEdgeMesh::getNormals() const {
    std::vector<glm::vec3> result;
    result.reserve(vertices.size());
    for (const auto& v : vertices) {
        result.push_back(v.normal);
    }
    return result;
}

std::vector<glm::vec2> HalfEdgeMesh::getUVs() const {
    std::vector<glm::vec2> result;
    result.reserve(vertices.size());
    for (const auto& v : vertices) {
        result.push_back(v.uv);
    }
    return result;
}

std::vector<std::vector<uint32_t>> HalfEdgeMesh::getFaceVertexIndices() const {
    std::vector<std::vector<uint32_t>> result;
    result.reserve(faces.size());
    for (uint32_t fi = 0; fi < static_cast<uint32_t>(faces.size()); ++fi) {
        result.push_back(faceVertices(fi));
    }
    return result;
}

void HalfEdgeMesh::setNormals(const std::vector<glm::vec3>& normals) {
    size_t count = std::min(normals.size(), vertices.size());
    for (size_t i = 0; i < count; ++i) {
        vertices[i].normal = normals[i];
    }
}

void HalfEdgeMesh::setUVs(const std::vector<glm::vec2>& uvs) {
    size_t count = std::min(uvs.size(), vertices.size());
    for (size_t i = 0; i < count; ++i) {
        vertices[i].uv = uvs[i];
    }
}

} // namespace PixellentModeler
