#include "PixellentModeler/Mesh/MeshBuffer.hpp"

#include <glad/glad.h>

namespace PixellentModeler {

// ---------------------------------------------------------------------------
// Destructor and move semantics
// ---------------------------------------------------------------------------

MeshBuffer::~MeshBuffer() {
    cleanup();
    cleanupWireframe();
}

MeshBuffer::MeshBuffer(MeshBuffer&& other) noexcept
    : m_vao(other.m_vao)
    , m_vbo(other.m_vbo)
    , m_ibo(other.m_ibo)
    , m_indexCount(other.m_indexCount)
    , m_wireVao(other.m_wireVao)
    , m_wireVbo(other.m_wireVbo)
    , m_wireIbo(other.m_wireIbo)
    , m_wireIndexCount(other.m_wireIndexCount)
{
    other.m_vao = 0;
    other.m_vbo = 0;
    other.m_ibo = 0;
    other.m_indexCount = 0;
    other.m_wireVao = 0;
    other.m_wireVbo = 0;
    other.m_wireIbo = 0;
    other.m_wireIndexCount = 0;
}

MeshBuffer& MeshBuffer::operator=(MeshBuffer&& other) noexcept {
    if (this != &other) {
        cleanup();
        cleanupWireframe();

        m_vao = other.m_vao;
        m_vbo = other.m_vbo;
        m_ibo = other.m_ibo;
        m_indexCount = other.m_indexCount;
        m_wireVao = other.m_wireVao;
        m_wireVbo = other.m_wireVbo;
        m_wireIbo = other.m_wireIbo;
        m_wireIndexCount = other.m_wireIndexCount;

        other.m_vao = 0;
        other.m_vbo = 0;
        other.m_ibo = 0;
        other.m_indexCount = 0;
        other.m_wireVao = 0;
        other.m_wireVbo = 0;
        other.m_wireIbo = 0;
        other.m_wireIndexCount = 0;
    }
    return *this;
}

// ---------------------------------------------------------------------------
// Upload solid mesh data
// ---------------------------------------------------------------------------

void MeshBuffer::upload(const TriMeshData& data) {
    cleanup();

    if (data.vertices.empty() || data.indices.empty())
        return;

    m_indexCount = static_cast<uint32_t>(data.indices.size());

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ibo);

    glBindVertexArray(m_vao);

    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(data.vertices.size() * sizeof(float)),
                 data.vertices.data(),
                 GL_STATIC_DRAW);

    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(data.indices.size() * sizeof(uint32_t)),
                 data.indices.data(),
                 GL_STATIC_DRAW);

    // Vertex layout: pos(3) + normal(3) + uv(2) = stride 8 floats
    const GLsizei stride = 8 * sizeof(float);

    // Location 0: position (vec3)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void*>(0));

    // Location 1: normal (vec3)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void*>(3 * sizeof(float)));

    // Location 2: uv (vec2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void*>(6 * sizeof(float)));

    glBindVertexArray(0);
}

// ---------------------------------------------------------------------------
// Upload wireframe data
// ---------------------------------------------------------------------------

void MeshBuffer::uploadWireframe(const HalfEdgeMesh::WireframeData& data) {
    cleanupWireframe();

    if (data.vertices.empty() || data.indices.empty())
        return;

    m_wireIndexCount = static_cast<uint32_t>(data.indices.size());

    glGenVertexArrays(1, &m_wireVao);
    glGenBuffers(1, &m_wireVbo);
    glGenBuffers(1, &m_wireIbo);

    glBindVertexArray(m_wireVao);

    // Upload vertex data (position only)
    glBindBuffer(GL_ARRAY_BUFFER, m_wireVbo);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(data.vertices.size() * sizeof(float)),
                 data.vertices.data(),
                 GL_STATIC_DRAW);

    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_wireIbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(data.indices.size() * sizeof(uint32_t)),
                 data.indices.data(),
                 GL_STATIC_DRAW);

    // Vertex layout: pos(3) only, stride = 3 floats
    const GLsizei stride = 3 * sizeof(float);

    // Location 0: position (vec3)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void*>(0));

    glBindVertexArray(0);
}

// ---------------------------------------------------------------------------
// Draw
// ---------------------------------------------------------------------------

void MeshBuffer::draw() const {
    if (m_vao == 0 || m_indexCount == 0)
        return;

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indexCount),
                   GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void MeshBuffer::drawWireframe() const {
    if (m_wireVao == 0 || m_wireIndexCount == 0)
        return;

    glBindVertexArray(m_wireVao);
    glDrawElements(GL_LINES, static_cast<GLsizei>(m_wireIndexCount),
                   GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

// ---------------------------------------------------------------------------
// Cleanup
// ---------------------------------------------------------------------------

void MeshBuffer::cleanup() {
    if (m_ibo) { glDeleteBuffers(1, &m_ibo); m_ibo = 0; }
    if (m_vbo) { glDeleteBuffers(1, &m_vbo); m_vbo = 0; }
    if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
    m_indexCount = 0;
}

void MeshBuffer::cleanupWireframe() {
    if (m_wireIbo) { glDeleteBuffers(1, &m_wireIbo); m_wireIbo = 0; }
    if (m_wireVbo) { glDeleteBuffers(1, &m_wireVbo); m_wireVbo = 0; }
    if (m_wireVao) { glDeleteVertexArrays(1, &m_wireVao); m_wireVao = 0; }
    m_wireIndexCount = 0;
}

} // namespace PixellentModeler
