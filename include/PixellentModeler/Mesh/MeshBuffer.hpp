#pragma once

#include <cstdint>

#include "PixellentModeler/Mesh/HalfEdgeMesh.hpp"

namespace PixellentModeler {

class MeshBuffer {
public:
    MeshBuffer() = default;
    ~MeshBuffer();

    // Non-copyable
    MeshBuffer(const MeshBuffer&) = delete;
    MeshBuffer& operator=(const MeshBuffer&) = delete;

    // Movable
    MeshBuffer(MeshBuffer&& other) noexcept;
    MeshBuffer& operator=(MeshBuffer&& other) noexcept;

    void upload(const TriMeshData& data);
    void uploadWireframe(const HalfEdgeMesh::WireframeData& data);
    void draw() const;
    void drawWireframe() const;

    bool hasData() const { return m_vao != 0; }
    bool hasWireframe() const { return m_wireVao != 0; }

private:
    // Solid mesh
    uint32_t m_vao = 0;
    uint32_t m_vbo = 0;
    uint32_t m_ibo = 0;
    uint32_t m_indexCount = 0;

    // Wireframe
    uint32_t m_wireVao = 0;
    uint32_t m_wireVbo = 0;
    uint32_t m_wireIbo = 0;
    uint32_t m_wireIndexCount = 0;

    void cleanup();
    void cleanupWireframe();
};

} // namespace PixellentModeler
