#pragma once

#include "PixellentModeler/Scene/Component.hpp"

namespace PixellentModeler {

class Scene;
class Entity;
class SelectionManager;

class SceneHierarchyPanel {
public:
    void render(Scene& scene, SelectionManager* selection);

    EntityID selectedEntity() const { return m_selectedEntity; }

private:
    EntityID m_selectedEntity = INVALID_ENTITY;
    void drawEntityNode(Entity& entity, Scene& scene, SelectionManager* selection);
};

} // namespace PixellentModeler
