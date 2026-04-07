#include "PixellentModeler/Mesh/MeshPrimitives.hpp"

#include <cmath>
#include <glm/gtc/constants.hpp>

namespace PixellentModeler {
namespace MeshPrimitives {

// ---------------------------------------------------------------------------
// Cube
// ---------------------------------------------------------------------------

std::unique_ptr<HalfEdgeMesh> createCube(float size) {
    auto mesh = std::make_unique<HalfEdgeMesh>();

    float h = size * 0.5f;

    // 8 vertices of a cube centered at origin
    std::vector<glm::vec3> positions = {
        {-h, -h,  h}, // 0: front-bottom-left
        { h, -h,  h}, // 1: front-bottom-right
        { h,  h,  h}, // 2: front-top-right
        {-h,  h,  h}, // 3: front-top-left
        {-h, -h, -h}, // 4: back-bottom-left
        { h, -h, -h}, // 5: back-bottom-right
        { h,  h, -h}, // 6: back-top-right
        {-h,  h, -h}, // 7: back-top-left
    };

    // 6 quad faces (vertices in CCW order when looking from outside)
    std::vector<std::vector<uint32_t>> faceIndices = {
        {0, 1, 2, 3}, // front  (+Z)
        {5, 4, 7, 6}, // back   (-Z)
        {3, 2, 6, 7}, // top    (+Y)
        {4, 5, 1, 0}, // bottom (-Y)
        {4, 0, 3, 7}, // left   (-X)
        {1, 5, 6, 2}, // right  (+X)
    };

    mesh->buildFromPolygons(positions, faceIndices);

    // Set basic UVs for each face (each face gets full 0-1 UV range)
    for (uint32_t fi = 0; fi < mesh->faceCount(); ++fi) {
        auto verts = mesh->faceVertices(fi);
        if (verts.size() == 4) {
            mesh->vertices[verts[0]].uv = {0.0f, 0.0f};
            mesh->vertices[verts[1]].uv = {1.0f, 0.0f};
            mesh->vertices[verts[2]].uv = {1.0f, 1.0f};
            mesh->vertices[verts[3]].uv = {0.0f, 1.0f};
        }
    }

    return mesh;
}

// ---------------------------------------------------------------------------
// Plane
// ---------------------------------------------------------------------------

std::unique_ptr<HalfEdgeMesh> createPlane(float size, int subdivisions) {
    auto mesh = std::make_unique<HalfEdgeMesh>();

    if (subdivisions < 1) subdivisions = 1;

    float h = size * 0.5f;
    int rows = subdivisions;
    int cols = subdivisions;
    int vertCols = cols + 1;
    int vertRows = rows + 1;

    // Generate vertices in a grid on the XZ plane, Y = 0
    std::vector<glm::vec3> positions;
    positions.reserve(static_cast<size_t>(vertRows) * vertCols);

    for (int r = 0; r < vertRows; ++r) {
        for (int c = 0; c < vertCols; ++c) {
            float x = -h + (size * c) / cols;
            float z = -h + (size * r) / rows;
            positions.push_back({x, 0.0f, z});
        }
    }

    // Generate quad faces
    std::vector<std::vector<uint32_t>> faceIndices;
    faceIndices.reserve(static_cast<size_t>(rows) * cols);

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            uint32_t bl = static_cast<uint32_t>(r * vertCols + c);
            uint32_t br = bl + 1;
            uint32_t tr = static_cast<uint32_t>((r + 1) * vertCols + c + 1);
            uint32_t tl = tr - 1;
            faceIndices.push_back({bl, br, tr, tl}); // CCW when viewed from +Y
        }
    }

    mesh->buildFromPolygons(positions, faceIndices);

    // Set UVs based on grid position
    for (int r = 0; r < vertRows; ++r) {
        for (int c = 0; c < vertCols; ++c) {
            uint32_t idx = static_cast<uint32_t>(r * vertCols + c);
            mesh->vertices[idx].uv = {
                static_cast<float>(c) / cols,
                static_cast<float>(r) / rows
            };
        }
    }

    return mesh;
}

// ---------------------------------------------------------------------------
// UV Sphere
// ---------------------------------------------------------------------------

std::unique_ptr<HalfEdgeMesh> createUVSphere(float radius, int segments, int rings) {
    auto mesh = std::make_unique<HalfEdgeMesh>();

    if (segments < 3) segments = 3;
    if (rings < 2) rings = 2;

    std::vector<glm::vec3> positions;

    // Top pole
    positions.push_back({0.0f, radius, 0.0f});

    // Body vertices (ring 1 to ring rings-1)
    for (int r = 1; r < rings; ++r) {
        float phi = glm::pi<float>() * static_cast<float>(r) / rings;
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);

        for (int s = 0; s < segments; ++s) {
            float theta = 2.0f * glm::pi<float>() * static_cast<float>(s) / segments;
            float x = radius * sinPhi * std::cos(theta);
            float y = radius * cosPhi;
            float z = radius * sinPhi * std::sin(theta);
            positions.push_back({x, y, z});
        }
    }

    // Bottom pole
    positions.push_back({0.0f, -radius, 0.0f});

    uint32_t topPole = 0;
    uint32_t bottomPole = static_cast<uint32_t>(positions.size() - 1);

    std::vector<std::vector<uint32_t>> faceIndices;

    // Top cap triangles (connecting top pole to first ring)
    for (int s = 0; s < segments; ++s) {
        uint32_t s0 = 1 + static_cast<uint32_t>(s);
        uint32_t s1 = 1 + static_cast<uint32_t>((s + 1) % segments);
        faceIndices.push_back({topPole, s0, s1});
    }

    // Body quads (ring i to ring i+1)
    for (int r = 0; r < rings - 2; ++r) {
        for (int s = 0; s < segments; ++s) {
            uint32_t current  = 1 + static_cast<uint32_t>(r * segments + s);
            uint32_t nextS    = 1 + static_cast<uint32_t>(r * segments + (s + 1) % segments);
            uint32_t below    = 1 + static_cast<uint32_t>((r + 1) * segments + s);
            uint32_t belowS   = 1 + static_cast<uint32_t>((r + 1) * segments + (s + 1) % segments);

            faceIndices.push_back({current, below, belowS, nextS});
        }
    }

    // Bottom cap triangles (connecting last ring to bottom pole)
    uint32_t lastRingStart = 1 + static_cast<uint32_t>((rings - 2) * segments);
    for (int s = 0; s < segments; ++s) {
        uint32_t s0 = lastRingStart + static_cast<uint32_t>(s);
        uint32_t s1 = lastRingStart + static_cast<uint32_t>((s + 1) % segments);
        faceIndices.push_back({s0, bottomPole, s1});
    }

    mesh->buildFromPolygons(positions, faceIndices);

    // Set UVs based on spherical coordinates
    // Top pole
    mesh->vertices[topPole].uv = {0.5f, 0.0f};

    for (int r = 1; r < rings; ++r) {
        float v = static_cast<float>(r) / rings;
        for (int s = 0; s < segments; ++s) {
            float u = static_cast<float>(s) / segments;
            uint32_t idx = 1 + static_cast<uint32_t>((r - 1) * segments + s);
            mesh->vertices[idx].uv = {u, v};
        }
    }

    // Bottom pole
    mesh->vertices[bottomPole].uv = {0.5f, 1.0f};

    return mesh;
}

// ---------------------------------------------------------------------------
// Cylinder
// ---------------------------------------------------------------------------

std::unique_ptr<HalfEdgeMesh> createCylinder(float radius, float height, int segments) {
    auto mesh = std::make_unique<HalfEdgeMesh>();

    if (segments < 3) segments = 3;

    float halfH = height * 0.5f;

    std::vector<glm::vec3> positions;

    // Bottom ring vertices (0 .. segments-1)
    for (int s = 0; s < segments; ++s) {
        float theta = 2.0f * glm::pi<float>() * static_cast<float>(s) / segments;
        float x = radius * std::cos(theta);
        float z = radius * std::sin(theta);
        positions.push_back({x, -halfH, z});
    }

    // Top ring vertices (segments .. 2*segments-1)
    for (int s = 0; s < segments; ++s) {
        float theta = 2.0f * glm::pi<float>() * static_cast<float>(s) / segments;
        float x = radius * std::cos(theta);
        float z = radius * std::sin(theta);
        positions.push_back({x, halfH, z});
    }

    // Center vertices for caps
    uint32_t bottomCenter = static_cast<uint32_t>(positions.size());
    positions.push_back({0.0f, -halfH, 0.0f});

    uint32_t topCenter = static_cast<uint32_t>(positions.size());
    positions.push_back({0.0f, halfH, 0.0f});

    std::vector<std::vector<uint32_t>> faceIndices;

    uint32_t bottomStart = 0;
    uint32_t topStart = static_cast<uint32_t>(segments);

    // Side quads
    for (int s = 0; s < segments; ++s) {
        uint32_t b0 = bottomStart + static_cast<uint32_t>(s);
        uint32_t b1 = bottomStart + static_cast<uint32_t>((s + 1) % segments);
        uint32_t t0 = topStart + static_cast<uint32_t>(s);
        uint32_t t1 = topStart + static_cast<uint32_t>((s + 1) % segments);
        // CCW when viewed from outside
        faceIndices.push_back({b1, b0, t0, t1});
    }

    // Bottom cap (n-gon, CCW when viewed from -Y)
    {
        std::vector<uint32_t> bottomFace;
        for (int s = segments - 1; s >= 0; --s) {
            bottomFace.push_back(bottomStart + static_cast<uint32_t>(s));
        }
        faceIndices.push_back(bottomFace);
    }

    // Top cap (n-gon, CCW when viewed from +Y)
    {
        std::vector<uint32_t> topFace;
        for (int s = 0; s < segments; ++s) {
            topFace.push_back(topStart + static_cast<uint32_t>(s));
        }
        faceIndices.push_back(topFace);
    }

    // Remove center vertices since we're using n-gon faces (not fan triangulation)
    positions.pop_back(); // remove topCenter
    positions.pop_back(); // remove bottomCenter

    mesh->buildFromPolygons(positions, faceIndices);

    // Set UVs
    for (int s = 0; s < segments; ++s) {
        float u = static_cast<float>(s) / segments;
        mesh->vertices[bottomStart + static_cast<uint32_t>(s)].uv = {u, 0.0f};
        mesh->vertices[topStart + static_cast<uint32_t>(s)].uv = {u, 1.0f};
    }

    return mesh;
}

// ---------------------------------------------------------------------------
// Cone
// ---------------------------------------------------------------------------

std::unique_ptr<HalfEdgeMesh> createCone(float radius, float height, int segments) {
    auto mesh = std::make_unique<HalfEdgeMesh>();

    if (segments < 3) segments = 3;

    float halfH = height * 0.5f;

    std::vector<glm::vec3> positions;

    // Bottom ring vertices (0 .. segments-1)
    for (int s = 0; s < segments; ++s) {
        float theta = 2.0f * glm::pi<float>() * static_cast<float>(s) / segments;
        float x = radius * std::cos(theta);
        float z = radius * std::sin(theta);
        positions.push_back({x, -halfH, z});
    }

    // Apex vertex
    uint32_t apex = static_cast<uint32_t>(positions.size());
    positions.push_back({0.0f, halfH, 0.0f});

    std::vector<std::vector<uint32_t>> faceIndices;

    // Side triangles
    for (int s = 0; s < segments; ++s) {
        uint32_t b0 = static_cast<uint32_t>(s);
        uint32_t b1 = static_cast<uint32_t>((s + 1) % segments);
        // CCW when viewed from outside
        faceIndices.push_back({b1, b0, apex});
    }

    // Bottom cap (n-gon, CCW when viewed from -Y)
    {
        std::vector<uint32_t> bottomFace;
        for (int s = segments - 1; s >= 0; --s) {
            bottomFace.push_back(static_cast<uint32_t>(s));
        }
        faceIndices.push_back(bottomFace);
    }

    mesh->buildFromPolygons(positions, faceIndices);

    // Set UVs
    for (int s = 0; s < segments; ++s) {
        float u = static_cast<float>(s) / segments;
        mesh->vertices[static_cast<uint32_t>(s)].uv = {u, 0.0f};
    }
    mesh->vertices[apex].uv = {0.5f, 1.0f};

    return mesh;
}

// ---------------------------------------------------------------------------
// Torus
// ---------------------------------------------------------------------------

std::unique_ptr<HalfEdgeMesh> createTorus(float majorRadius, float minorRadius,
                                          int majorSegments, int minorSegments) {
    auto mesh = std::make_unique<HalfEdgeMesh>();

    if (majorSegments < 3) majorSegments = 3;
    if (minorSegments < 3) minorSegments = 3;

    std::vector<glm::vec3> positions;
    positions.reserve(static_cast<size_t>(majorSegments) * minorSegments);

    // Generate vertices using parametric torus equations
    for (int i = 0; i < majorSegments; ++i) {
        float theta = 2.0f * glm::pi<float>() * static_cast<float>(i) / majorSegments;
        float cosTheta = std::cos(theta);
        float sinTheta = std::sin(theta);

        for (int j = 0; j < minorSegments; ++j) {
            float phi = 2.0f * glm::pi<float>() * static_cast<float>(j) / minorSegments;
            float cosPhi = std::cos(phi);
            float sinPhi = std::sin(phi);

            float x = (majorRadius + minorRadius * cosPhi) * cosTheta;
            float y = minorRadius * sinPhi;
            float z = (majorRadius + minorRadius * cosPhi) * sinTheta;

            positions.push_back({x, y, z});
        }
    }

    // Generate quad faces
    std::vector<std::vector<uint32_t>> faceIndices;
    faceIndices.reserve(static_cast<size_t>(majorSegments) * minorSegments);

    for (int i = 0; i < majorSegments; ++i) {
        int nextI = (i + 1) % majorSegments;
        for (int j = 0; j < minorSegments; ++j) {
            int nextJ = (j + 1) % minorSegments;

            uint32_t v0 = static_cast<uint32_t>(i * minorSegments + j);
            uint32_t v1 = static_cast<uint32_t>(nextI * minorSegments + j);
            uint32_t v2 = static_cast<uint32_t>(nextI * minorSegments + nextJ);
            uint32_t v3 = static_cast<uint32_t>(i * minorSegments + nextJ);

            faceIndices.push_back({v0, v1, v2, v3});
        }
    }

    mesh->buildFromPolygons(positions, faceIndices);

    // Set UVs
    for (int i = 0; i < majorSegments; ++i) {
        float u = static_cast<float>(i) / majorSegments;
        for (int j = 0; j < minorSegments; ++j) {
            float v = static_cast<float>(j) / minorSegments;
            uint32_t idx = static_cast<uint32_t>(i * minorSegments + j);
            mesh->vertices[idx].uv = {u, v};
        }
    }

    return mesh;
}

} // namespace MeshPrimitives
} // namespace PixellentModeler
