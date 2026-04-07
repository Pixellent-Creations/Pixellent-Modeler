#pragma once

#include "PixellentModeler/Core/EventDispatcher.hpp"
#include <glm/glm.hpp>
#include <string>
#include <functional>
#include <unordered_map>
#include <vector>

namespace PixellentModeler {

class InputManager {
public:
    explicit InputManager(EventDispatcher& dispatcher);

    void beginFrame();
    void endFrame();

    bool isKeyDown(int key) const;
    bool isKeyPressed(int key) const;  // just this frame
    bool isMouseButtonDown(int button) const;
    bool isMouseButtonPressed(int button) const;
    glm::vec2 mousePosition() const;
    glm::vec2 mouseDelta() const;
    float scrollDelta() const;

    using ShortcutCallback = std::function<void()>;
    void registerShortcut(const std::string& name, int key, int mods, ShortcutCallback cb);

private:
    EventDispatcher& m_dispatcher;
    std::unordered_map<int, bool> m_keyState;
    std::unordered_map<int, bool> m_prevKeyState;
    std::unordered_map<int, bool> m_mouseState;
    std::unordered_map<int, bool> m_prevMouseState;
    glm::vec2 m_mousePos{0.0f};
    glm::vec2 m_prevMousePos{0.0f};
    float m_scrollDelta = 0.0f;

    struct Shortcut {
        std::string name;
        int key;
        int mods;
        ShortcutCallback callback;
    };
    std::vector<Shortcut> m_shortcuts;

    void onKeyEvent(Event& event);
    void onMouseButtonEvent(Event& event);
    void onMouseMoveEvent(Event& event);
    void onMouseScrollEvent(Event& event);
};

} // namespace PixellentModeler
