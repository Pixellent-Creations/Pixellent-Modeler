#include "PixellentModeler/Scene/Scene.hpp"
#include "PixellentModeler/Scene/TransformComponent.hpp"
#include "PixellentModeler/Scene/MeshComponent.hpp"
#include "PixellentModeler/Scene/MaterialComponent.hpp"
#include "PixellentModeler/Scene/LightComponent.hpp"
#include "PixellentModeler/Mesh/HalfEdgeMesh.hpp"
#include "PixellentModeler/Mesh/MeshBuffer.hpp"
#include <algorithm>

namespace PixellentModeler {

Scene::Scene()
    : m_camera(45.0f, 16.0f / 9.0f) {
}

Scene::~Scene() = default;

Entity& Scene::createEntity(const std::string& name) {
    auto entity = std::make_unique<Entity>(m_nextId++, name);
    Entity& ref = *entity;
    m_entities.push_back(std::move(entity));
    return ref;
}

void Scene::destroyEntity(EntityID id) {
    auto it = std::find_if(m_entities.begin(), m_entities.end(),
        [id](const std::unique_ptr<Entity>& e) { return e->id() == id; });

    if (it != m_entities.end()) {
        // Remove from parent if any
        Entity* entity = it->get();
        if (entity->parent()) {
            entity->parent()->removeChild(entity);
        }

        // Detach children from this entity
        const auto& children = entity->children();
        for (Entity* child : children) {
            child->setParent(nullptr);
        }

        m_entities.erase(it);
    }
}

Entity* Scene::getEntity(EntityID id) {
    auto it = std::find_if(m_entities.begin(), m_entities.end(),
        [id](const std::unique_ptr<Entity>& e) { return e->id() == id; });

    return it != m_entities.end() ? it->get() : nullptr;
}

void Scene::clear() {
    m_entities.clear();
    m_nextId = 0;
}

void Scene::duplicateEntity(EntityID id) {
    Entity* source = getEntity(id);
    if (!source) return;

    Entity& copy = createEntity(source->name() + " Copy");

    // Clone TransformComponent
    if (auto* tc = source->getComponent<TransformComponent>()) {
        auto& newTc = copy.addComponent<TransformComponent>();
        newTc.position = tc->position;
        newTc.rotation = tc->rotation;
        newTc.scale = tc->scale;
    }

    // Clone MeshComponent (deep copy of mesh data)
    if (auto* mc = source->getComponent<MeshComponent>()) {
        auto& newMc = copy.addComponent<MeshComponent>();
        if (mc->mesh) {
            newMc.mesh = std::make_shared<HalfEdgeMesh>(*mc->mesh);
        }
        newMc.buffer = std::make_shared<MeshBuffer>();
        newMc.rebuild();
    }

    // Clone MaterialComponent
    if (auto* mat = source->getComponent<MaterialComponent>()) {
        auto& newMat = copy.addComponent<MaterialComponent>();
        newMat.albedo = mat->albedo;
        newMat.metallic = mat->metallic;
        newMat.roughness = mat->roughness;
    }

    // Clone LightComponent
    if (auto* lc = source->getComponent<LightComponent>()) {
        auto& newLc = copy.addComponent<LightComponent>();
        newLc.type = lc->type;
        newLc.color = lc->color;
        newLc.intensity = lc->intensity;
        newLc.direction = lc->direction;
    }
}

} // namespace PixellentModeler
