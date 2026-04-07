#include "PixellentModeler/Rendering/VertexArray.hpp"
#include "PixellentModeler/Rendering/VertexBuffer.hpp"
#include "PixellentModeler/Rendering/IndexBuffer.hpp"
#include <glad/glad.h>

namespace PixellentModeler {

VertexArray::VertexArray() {
    glGenVertexArrays(1, &m_id);
}

VertexArray::~VertexArray() {
    if (m_id) {
        glDeleteVertexArrays(1, &m_id);
    }
}

void VertexArray::bind() const {
    glBindVertexArray(m_id);
}

void VertexArray::unbind() const {
    glBindVertexArray(0);
}

void VertexArray::addVertexBuffer(std::shared_ptr<VertexBuffer> vb, const std::vector<VertexAttribute>& layout) {
    glBindVertexArray(m_id);
    vb->bind();

    for (const auto& attr : layout) {
        glEnableVertexAttribArray(attr.index);
        glVertexAttribPointer(
            attr.index,
            attr.size,
            attr.type,
            attr.normalized ? GL_TRUE : GL_FALSE,
            static_cast<GLsizei>(attr.stride),
            reinterpret_cast<const void*>(attr.offset)
        );
    }

    m_vertexBuffers.push_back(std::move(vb));
    glBindVertexArray(0);
}

void VertexArray::setIndexBuffer(std::shared_ptr<IndexBuffer> ib) {
    glBindVertexArray(m_id);
    ib->bind();
    m_indexBuffer = std::move(ib);
    glBindVertexArray(0);
}

} // namespace PixellentModeler
