#pragma once

#include <vector>
#include <cstdint>
#include <glm/glm.hpp>

#include "PixellentModeler/Selection/BoundingBox.hpp"

namespace PixellentModeler {

static constexpr uint32_t INVALID_INDEX = UINT32_MAX;

struct HEVertex {
    glm::vec3 position{0};
    glm::vec3 normal{0};
    glm::vec2 uv{0};
    uint32_t halfEdge = INVALID_INDEX; // one outgoing half-edge
    bool selected = false;
};

struct HEHalfEdge {
    uint32_t vertex = INVALID_INDEX;  // vertex this half-edge POINTS TO
    uint32_t face = INVALID_INDEX;    // face to the left (INVALID if boundary)
    uint32_t next = INVALID_INDEX;    // next half-edge around the face (CCW)
    uint32_t prev = INVALID_INDEX;    // previous half-edge
    uint32_t twin = INVALID_INDEX;    // opposite half-edge
};

struct HEEdge {
    uint32_t halfEdge = INVALID_INDEX; // one of the two half-edges
    bool selected = false;
};

struct HEFace {
    uint32_t halfEdge = INVALID_INDEX; // any half-edge on this face
    glm::vec3 normal{0};
    bool selected = false;
};

struct TriMeshData {
    std::vector<float> vertices;     // pos(3) + normal(3) + uv(2) = stride 8
    std::vector<uint32_t> indices;
};

class HalfEdgeMesh {
public:
    std::vector<HEVertex> vertices;
    std::vector<HEHalfEdge> halfEdges;
    std::vector<HEEdge> edges;
    std::vector<HEFace> faces;

    // Construction
    uint32_t addVertex(const glm::vec3& pos);
    uint32_t addFace(const std::vector<uint32_t>& vertexIndices);

    // Build from triangles (for importing)
    void buildFromTriangles(const std::vector<glm::vec3>& positions,
                            const std::vector<uint32_t>& indices);

    // Build from quads/polygons
    void buildFromPolygons(const std::vector<glm::vec3>& positions,
                           const std::vector<std::vector<uint32_t>>& faceIndices);

    // Topology queries
    std::vector<uint32_t> vertexFaces(uint32_t vertId) const;
    std::vector<uint32_t> vertexNeighbors(uint32_t vertId) const;
    std::vector<uint32_t> vertexEdges(uint32_t vertId) const;
    std::vector<uint32_t> faceVertices(uint32_t faceId) const;
    std::vector<uint32_t> faceEdges(uint32_t faceId) const;
    std::vector<uint32_t> edgeFaces(uint32_t edgeId) const;
    uint32_t edgeVertex0(uint32_t edgeId) const;
    uint32_t edgeVertex1(uint32_t edgeId) const;
    bool isBoundaryVertex(uint32_t vertId) const;
    bool isBoundaryEdge(uint32_t edgeId) const;

    // Geometry
    void recomputeNormals();
    BoundingBox computeBounds() const;

    // GPU upload
    TriMeshData triangulate() const; // fan-triangulate each face

    // Wireframe data
    struct WireframeData {
        std::vector<float> vertices;   // pos(3) per vertex
        std::vector<uint32_t> indices; // line indices
    };
    WireframeData wireframeData() const;

    // Validation
    bool validate() const;

    // Stats
    uint32_t vertexCount() const { return static_cast<uint32_t>(vertices.size()); }
    uint32_t edgeCount() const { return static_cast<uint32_t>(edges.size()); }
    uint32_t faceCount() const { return static_cast<uint32_t>(faces.size()); }

    // Convenience accessors for serialization / IO
    std::vector<glm::vec3> getVertices() const;
    std::vector<glm::vec3> getNormals() const;
    std::vector<glm::vec2> getUVs() const;
    std::vector<std::vector<uint32_t>> getFaceVertexIndices() const;
    void setNormals(const std::vector<glm::vec3>& normals);
    void setUVs(const std::vector<glm::vec2>& uvs);

    void clear();
};

} // namespace PixellentModeler
