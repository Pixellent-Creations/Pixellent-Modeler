#include "PixellentModeler/Rendering/Framebuffer.hpp"
#include <glad/glad.h>
#include <iostream>

namespace PixellentModeler {

Framebuffer::Framebuffer(const FramebufferSpec& spec)
    : m_spec(spec) {
    create();
}

Framebuffer::~Framebuffer() {
    destroy();
}

void Framebuffer::create() {
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Create color attachments
    m_colorAttachments.resize(static_cast<size_t>(m_spec.colorAttachments));
    if (m_spec.colorAttachments > 0) {
        glGenTextures(m_spec.colorAttachments, m_colorAttachments.data());

        for (int i = 0; i < m_spec.colorAttachments; ++i) {
            glBindTexture(GL_TEXTURE_2D, m_colorAttachments[static_cast<size_t>(i)]);

            // Use RGBA8 for the first attachment, R32I for additional attachments (useful for picking)
            if (i == 0) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_spec.width, m_spec.height,
                             0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            } else {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, m_spec.width, m_spec.height,
                             0, GL_RED_INTEGER, GL_INT, nullptr);
            }

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                                   GL_TEXTURE_2D, m_colorAttachments[static_cast<size_t>(i)], 0);
        }
    }

    // Set draw buffers
    if (m_spec.colorAttachments > 1) {
        std::vector<GLenum> drawBuffers(static_cast<size_t>(m_spec.colorAttachments));
        for (int i = 0; i < m_spec.colorAttachments; ++i) {
            drawBuffers[static_cast<size_t>(i)] = GL_COLOR_ATTACHMENT0 + i;
        }
        glDrawBuffers(m_spec.colorAttachments, drawBuffers.data());
    }

    // Create depth attachment
    if (m_spec.depthAttachment) {
        glGenRenderbuffers(1, &m_depthAttachment);
        glBindRenderbuffer(GL_RENDERBUFFER, m_depthAttachment);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_spec.width, m_spec.height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                                  GL_RENDERBUFFER, m_depthAttachment);
    }

    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer is not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::destroy() {
    if (m_fbo) {
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }
    if (!m_colorAttachments.empty()) {
        glDeleteTextures(static_cast<GLsizei>(m_colorAttachments.size()), m_colorAttachments.data());
        m_colorAttachments.clear();
    }
    if (m_depthAttachment) {
        glDeleteRenderbuffers(1, &m_depthAttachment);
        m_depthAttachment = 0;
    }
}

void Framebuffer::bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_spec.width, m_spec.height);
}

void Framebuffer::unbind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::resize(int width, int height) {
    if (width <= 0 || height <= 0) return;
    m_spec.width = width;
    m_spec.height = height;
    destroy();
    create();
}

uint32_t Framebuffer::colorAttachmentID(int index) const {
    if (index < 0 || index >= static_cast<int>(m_colorAttachments.size())) {
        return 0;
    }
    return m_colorAttachments[static_cast<size_t>(index)];
}

int Framebuffer::readPixel(int attachmentIndex, int x, int y) const {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
    int pixelData = -1;
    glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    return pixelData;
}

} // namespace PixellentModeler
