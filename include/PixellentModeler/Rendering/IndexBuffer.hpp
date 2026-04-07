#pragma once

#include <cstdint>

namespace PixellentModeler {

class IndexBuffer {
public:
    IndexBuffer(const uint32_t* data, uint32_t count);
    ~IndexBuffer();

    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;
    IndexBuffer(IndexBuffer&& other) noexcept;
    IndexBuffer& operator=(IndexBuffer&& other) noexcept;

    void bind() const;
    void unbind() const;
    uint32_t count() const { return m_count; }

private:
    uint32_t m_id = 0;
    uint32_t m_count = 0;
};

} // namespace PixellentModeler
