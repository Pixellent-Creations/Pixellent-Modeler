#pragma once

#include "PixellentModeler/Core/Event.hpp"
#include <functional>
#include <unordered_map>
#include <vector>

namespace PixellentModeler {

class EventDispatcher {
public:
    using Callback = std::function<void(Event&)>;

    void subscribe(EventType type, Callback cb);
    void dispatch(Event& event);

private:
    std::unordered_map<EventType, std::vector<Callback>> m_listeners;
};

} // namespace PixellentModeler
