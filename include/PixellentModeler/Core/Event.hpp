#pragma once

namespace PixellentModeler {

enum class EventType {
    None = 0,
    WindowClose,
    WindowResize,
    WindowFocus,
    KeyPressed,
    KeyReleased,
    KeyTyped,
    MouseButtonPressed,
    MouseButtonReleased,
    MouseMoved,
    MouseScrolled,
    SceneChanged,
    SelectionChanged,
    ToolChanged,
    EntityCreated,
    EntityDeleted,
    EntityTransformed,
    UndoPerformed,
    RedoPerformed
};

struct Event {
    EventType type = EventType::None;
    bool handled = false;

    explicit Event(EventType t) : type(t) {}
    virtual ~Event() = default;
};

struct WindowResizeEvent : public Event {
    int width;
    int height;

    WindowResizeEvent(int w, int h)
        : Event(EventType::WindowResize), width(w), height(h) {}
};

struct WindowCloseEvent : public Event {
    WindowCloseEvent()
        : Event(EventType::WindowClose) {}
};

struct KeyEvent : public Event {
    int key;
    int scancode;
    int action;
    int mods;

    KeyEvent(EventType type, int key, int scancode, int action, int mods)
        : Event(type), key(key), scancode(scancode), action(action), mods(mods) {}
};

struct MouseButtonEvent : public Event {
    int button;
    int action;
    int mods;

    MouseButtonEvent(EventType type, int button, int action, int mods)
        : Event(type), button(button), action(action), mods(mods) {}
};

struct MouseMoveEvent : public Event {
    double x;
    double y;

    MouseMoveEvent(double x, double y)
        : Event(EventType::MouseMoved), x(x), y(y) {}
};

struct MouseScrollEvent : public Event {
    double xOffset;
    double yOffset;

    MouseScrollEvent(double xOff, double yOff)
        : Event(EventType::MouseScrolled), xOffset(xOff), yOffset(yOff) {}
};

struct SelectionChangedEvent : public Event {
    SelectionChangedEvent()
        : Event(EventType::SelectionChanged) {}
};

} // namespace PixellentModeler
