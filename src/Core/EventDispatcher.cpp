#include "PixellentModeler/Core/EventDispatcher.hpp"

namespace PixellentModeler {

void EventDispatcher::subscribe(EventType type, Callback cb) {
    m_listeners[type].push_back(std::move(cb));
}

void EventDispatcher::dispatch(Event& event) {
    auto it = m_listeners.find(event.type);
    if (it == m_listeners.end()) {
        return;
    }

    for (auto& callback : it->second) {
        callback(event);
        if (event.handled) {
            break;
        }
    }
}

} // namespace PixellentModeler
