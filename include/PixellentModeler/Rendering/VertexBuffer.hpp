#pragma once

#include <cstdint>

namespace PixellentModeler {

class VertexBuffer {
public:
    VertexBuffer(const void* data, uint32_t size);
    ~VertexBuffer();

    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;
    VertexBuffer(VertexBuffer&& other) noexcept;
    VertexBuffer& operator=(VertexBuffer&& other) noexcept;

    void bind() const;
    void unbind() const;
    void updateData(const void* data, uint32_t size);

private:
    uint32_t m_id = 0;
};

} // namespace PixellentModeler
