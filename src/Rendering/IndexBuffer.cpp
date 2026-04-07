#include "PixellentModeler/Rendering/IndexBuffer.hpp"
#include <glad/glad.h>
#include <utility>

namespace PixellentModeler {

IndexBuffer::IndexBuffer(const uint32_t* data, uint32_t count)
    : m_count(count) {
    glGenBuffers(1, &m_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), data, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

IndexBuffer::~IndexBuffer() {
    if (m_id) {
        glDeleteBuffers(1, &m_id);
    }
}

IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept
    : m_id(other.m_id), m_count(other.m_count) {
    other.m_id = 0;
    other.m_count = 0;
}

IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept {
    if (this != &other) {
        if (m_id) {
            glDeleteBuffers(1, &m_id);
        }
        m_id = other.m_id;
        m_count = other.m_count;
        other.m_id = 0;
        other.m_count = 0;
    }
    return *this;
}

void IndexBuffer::bind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
}

void IndexBuffer::unbind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

} // namespace PixellentModeler
