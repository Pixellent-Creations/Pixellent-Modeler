#pragma once
#include <cstdint>

namespace PixellentModeler {

using EntityID = uint32_t;
static constexpr EntityID INVALID_ENTITY = UINT32_MAX;

class Entity;

class Component {
public:
    virtual ~Component() = default;
    EntityID owner = INVALID_ENTITY;
    virtual void onAttach(Entity& entity) {}
    virtual void onDetach() {}
};

} // namespace PixellentModeler
