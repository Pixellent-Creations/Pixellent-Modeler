#pragma once
#include "PixellentModeler/Scene/Entity.hpp"
#include "PixellentModeler/Scene/Camera.hpp"
#include <vector>
#include <memory>
#include <string>
#include <glm/glm.hpp>

namespace PixellentModeler {

class Scene {
public:
    Scene();
    ~Scene();

    Entity& createEntity(const std::string& name = "Entity");
    void destroyEntity(EntityID id);
    Entity* getEntity(EntityID id);
    Entity* findEntity(EntityID id) { return getEntity(id); }
    void duplicateEntity(EntityID id);
    const std::vector<std::unique_ptr<Entity>>& entities() const { return m_entities; }
    const std::vector<std::unique_ptr<Entity>>& getEntities() const { return m_entities; }

    Camera& camera() { return m_camera; }
    const Camera& camera() const { return m_camera; }

    void clear();  // remove all entities

    // Scene-level properties
    glm::vec3 ambientLight{0.1f, 0.1f, 0.1f};

private:
    std::vector<std::unique_ptr<Entity>> m_entities;
    EntityID m_nextId = 0;
    Camera m_camera;
};

} // namespace PixellentModeler
