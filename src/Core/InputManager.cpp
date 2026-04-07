#include "PixellentModeler/Core/InputManager.hpp"
#include <GLFW/glfw3.h>

namespace PixellentModeler {

InputManager::InputManager(EventDispatcher& dispatcher)
    : m_dispatcher(dispatcher)
{
    m_dispatcher.subscribe(EventType::KeyPressed, [this](Event& e) { onKeyEvent(e); });
    m_dispatcher.subscribe(EventType::KeyReleased, [this](Event& e) { onKeyEvent(e); });
    m_dispatcher.subscribe(EventType::MouseButtonPressed, [this](Event& e) { onMouseButtonEvent(e); });
    m_dispatcher.subscribe(EventType::MouseButtonReleased, [this](Event& e) { onMouseButtonEvent(e); });
    m_dispatcher.subscribe(EventType::MouseMoved, [this](Event& e) { onMouseMoveEvent(e); });
    m_dispatcher.subscribe(EventType::MouseScrolled, [this](Event& e) { onMouseScrollEvent(e); });
}

void InputManager::beginFrame() {
    m_prevKeyState = m_keyState;
    m_prevMouseState = m_mouseState;
    m_prevMousePos = m_mousePos;
    m_scrollDelta = 0.0f;
}

void InputManager::endFrame() {
    // Nothing to do currently; state has already been captured in beginFrame
}

bool InputManager::isKeyDown(int key) const {
    auto it = m_keyState.find(key);
    return it != m_keyState.end() && it->second;
}

bool InputManager::isKeyPressed(int key) const {
    bool current = isKeyDown(key);
    auto it = m_prevKeyState.find(key);
    bool previous = (it != m_prevKeyState.end()) && it->second;
    return current && !previous;
}

bool InputManager::isMouseButtonDown(int button) const {
    auto it = m_mouseState.find(button);
    return it != m_mouseState.end() && it->second;
}

bool InputManager::isMouseButtonPressed(int button) const {
    bool current = isMouseButtonDown(button);
    auto it = m_prevMouseState.find(button);
    bool previous = (it != m_prevMouseState.end()) && it->second;
    return current && !previous;
}

glm::vec2 InputManager::mousePosition() const {
    return m_mousePos;
}

glm::vec2 InputManager::mouseDelta() const {
    return m_mousePos - m_prevMousePos;
}

float InputManager::scrollDelta() const {
    return m_scrollDelta;
}

void InputManager::registerShortcut(const std::string& name, int key, int mods, ShortcutCallback cb) {
    m_shortcuts.push_back({name, key, mods, std::move(cb)});
}

void InputManager::onKeyEvent(Event& event) {
    auto& keyEvent = static_cast<KeyEvent&>(event);

    if (keyEvent.action == GLFW_PRESS) {
        m_keyState[keyEvent.key] = true;

        // Check shortcuts on key press
        for (auto& shortcut : m_shortcuts) {
            if (keyEvent.key == shortcut.key && (keyEvent.mods & shortcut.mods) == shortcut.mods) {
                shortcut.callback();
                event.handled = true;
                break;
            }
        }
    } else if (keyEvent.action == GLFW_RELEASE) {
        m_keyState[keyEvent.key] = false;
    }
}

void InputManager::onMouseButtonEvent(Event& event) {
    auto& mouseEvent = static_cast<MouseButtonEvent&>(event);

    if (mouseEvent.action == GLFW_PRESS) {
        m_mouseState[mouseEvent.button] = true;
    } else if (mouseEvent.action == GLFW_RELEASE) {
        m_mouseState[mouseEvent.button] = false;
    }
}

void InputManager::onMouseMoveEvent(Event& event) {
    auto& moveEvent = static_cast<MouseMoveEvent&>(event);
    m_mousePos = glm::vec2(static_cast<float>(moveEvent.x), static_cast<float>(moveEvent.y));
}

void InputManager::onMouseScrollEvent(Event& event) {
    auto& scrollEvent = static_cast<MouseScrollEvent&>(event);
    m_scrollDelta += static_cast<float>(scrollEvent.yOffset);
}

} // namespace PixellentModeler
