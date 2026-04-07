#pragma once

#include <memory>
#include <glm/glm.hpp>
#include "PixellentModeler/Scene/Component.hpp"

namespace PixellentModeler {

class Framebuffer;
class Shader;
class MeshBuffer;

class Picker {
public:
    Picker() = default;
    void init(int width, int height);
    void resize(int width, int height);

    void beginPick();
    void renderEntityForPick(MeshBuffer& meshBuffer, const glm::mat4& mvp, EntityID entityId);
    void endPick();

    EntityID pickEntity(int mouseX, int mouseY) const;

    Framebuffer* framebuffer() { return m_framebuffer.get(); }

private:
    std::unique_ptr<Framebuffer> m_framebuffer;
    std::unique_ptr<Shader> m_pickShader;
    int m_width = 800;
    int m_height = 600;
};

} // namespace PixellentModeler
