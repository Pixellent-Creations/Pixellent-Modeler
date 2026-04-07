#pragma once

#include <string>
#include <glm/glm.hpp>

namespace PixellentModeler {

class Renderer;
class Scene;
class SelectionManager;
class InputManager;
class CommandHistory;
class Camera;

struct ToolContext {
    Scene* scene = nullptr;
    Renderer* renderer = nullptr;
    SelectionManager* selection = nullptr;
    InputManager* input = nullptr;
    CommandHistory* commands = nullptr;
    Camera* camera = nullptr;
    glm::vec2 mousePos{0};
    glm::vec2 viewportSize{800, 600};
};

class Tool {
public:
    virtual ~Tool() = default;
    virtual std::string name() const = 0;
    virtual std::string shortcut() const { return ""; }

    virtual void onActivate() {}
    virtual void onDeactivate() {}
    virtual void onMouseDown(const ToolContext& ctx, int button, int mods) {}
    virtual void onMouseMove(const ToolContext& ctx) {}
    virtual void onMouseUp(const ToolContext& ctx, int button, int mods) {}
    virtual void onKeyDown(const ToolContext& ctx, int key, int mods) {}
    virtual void onRender(const ToolContext& ctx) {}

    bool isActive() const { return m_active; }
    void setActive(bool active) { m_active = active; }

protected:
    bool m_active = false;
};

} // namespace PixellentModeler
