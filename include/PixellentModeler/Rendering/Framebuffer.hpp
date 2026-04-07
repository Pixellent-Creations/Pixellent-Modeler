#pragma once

#include <cstdint>
#include <vector>

namespace PixellentModeler {

struct FramebufferSpec {
    int width = 800;
    int height = 600;
    bool depthAttachment = true;
    int colorAttachments = 1;  // number of color attachments
};

class Framebuffer {
public:
    Framebuffer(const FramebufferSpec& spec);
    ~Framebuffer();

    void bind() const;
    void unbind() const;
    void resize(int width, int height);
    uint32_t colorAttachmentID(int index = 0) const;
    int readPixel(int attachmentIndex, int x, int y) const;

    int width() const { return m_spec.width; }
    int height() const { return m_spec.height; }
    uint32_t fbo() const { return m_fbo; }
    const FramebufferSpec& spec() const { return m_spec; }

private:
    void create();
    void destroy();

    uint32_t m_fbo = 0;
    std::vector<uint32_t> m_colorAttachments;
    uint32_t m_depthAttachment = 0;
    FramebufferSpec m_spec;
};

} // namespace PixellentModeler
