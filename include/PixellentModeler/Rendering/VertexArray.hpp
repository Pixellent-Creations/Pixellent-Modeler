#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace PixellentModeler {

class VertexBuffer;
class IndexBuffer;

struct VertexAttribute {
    uint32_t index;
    int size;        // number of components (1, 2, 3, 4)
    uint32_t type;   // GL_FLOAT, etc.
    bool normalized;
    uint32_t stride;
    uint64_t offset;
};

class VertexArray {
public:
    VertexArray();
    ~VertexArray();

    VertexArray(const VertexArray&) = delete;
    VertexArray& operator=(const VertexArray&) = delete;

    void bind() const;
    void unbind() const;
    void addVertexBuffer(std::shared_ptr<VertexBuffer> vb, const std::vector<VertexAttribute>& layout);
    void setIndexBuffer(std::shared_ptr<IndexBuffer> ib);
    const std::shared_ptr<IndexBuffer>& indexBuffer() const { return m_indexBuffer; }

private:
    uint32_t m_id = 0;
    std::vector<std::shared_ptr<VertexBuffer>> m_vertexBuffers;
    std::shared_ptr<IndexBuffer> m_indexBuffer;
};

} // namespace PixellentModeler
