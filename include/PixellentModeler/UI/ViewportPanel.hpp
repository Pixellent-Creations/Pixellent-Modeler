#pragma once

#include <memory>
#include <glm/glm.hpp>
#include "PixellentModeler/Rendering/Framebuffer.hpp"

namespace PixellentModeler {

class Renderer;
class Scene;

class ViewportPanel {
public:
    void init();
    void render(Renderer& renderer, Scene& scene);

    bool isHovered() const { return m_hovered; }
    bool isFocused() const { return m_focused; }
    glm::vec2 viewportSize() const { return m_viewportSize; }
    glm::vec2 viewportPos() const { return m_viewportPos; }

private:
    bool m_hovered = false;
    bool m_focused = false;
    glm::vec2 m_viewportSize{800, 600};
    glm::vec2 m_viewportPos{0, 0};
    std::unique_ptr<Framebuffer> m_framebuffer;
};

} // namespace PixellentModeler
