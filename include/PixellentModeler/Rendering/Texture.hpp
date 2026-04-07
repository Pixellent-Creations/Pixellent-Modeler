#pragma once

#include <cstdint>
#include <string>

namespace PixellentModeler {

class Texture {
public:
    Texture() = default;
    ~Texture();

    bool loadFromFile(const std::string& path);
    bool create(int width, int height, const unsigned char* data, int channels = 4);
    void bind(uint32_t slot = 0) const;
    void unbind() const;

    int width() const { return m_width; }
    int height() const { return m_height; }
    uint32_t id() const { return m_id; }

private:
    uint32_t m_id = 0;
    int m_width = 0;
    int m_height = 0;
    int m_channels = 0;
};

} // namespace PixellentModeler
