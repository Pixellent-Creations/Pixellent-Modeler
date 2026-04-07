#include "PixellentModeler/Selection/SelectionManager.hpp"
#include "PixellentModeler/Scene/Scene.hpp"
#include "PixellentModeler/Scene/Entity.hpp"
#include "PixellentModeler/Scene/TransformComponent.hpp"

namespace PixellentModeler {

// ---------------------------------------------------------------------------
// Selection
// ---------------------------------------------------------------------------

void Selection::clear() {
    entities.clear();
    vertices.clear();
    edges.clear();
    faces.clear();
}

bool Selection::isEmpty() const {
    return entities.empty() && vertices.empty() && edges.empty() && faces.empty();
}

bool Selection::isEntitySelected(EntityID id) const {
    return entities.find(id) != entities.end();
}

// ---------------------------------------------------------------------------
// SelectionManager
// ---------------------------------------------------------------------------

SelectionManager::SelectionManager(EventDispatcher* dispatcher)
    : m_dispatcher(dispatcher) {
}

void SelectionManager::setMode(SelectionMode mode) {
    if (m_mode == mode) return;

    // When switching modes, clear sub-object selections that don't belong
    // to the new mode, but keep object-level selection intact.
    m_mode = mode;
    m_selection.mode = mode;

    switch (mode) {
        case SelectionMode::Object:
            m_selection.vertices.clear();
            m_selection.edges.clear();
            m_selection.faces.clear();
            break;
        case SelectionMode::Vertex:
            m_selection.edges.clear();
            m_selection.faces.clear();
            break;
        case SelectionMode::Edge:
            m_selection.vertices.clear();
            m_selection.faces.clear();
            break;
        case SelectionMode::Face:
            m_selection.vertices.clear();
            m_selection.edges.clear();
            break;
    }

    notifyChanged();
}

void SelectionManager::select(EntityID entity) {
    m_selection.clear();
    m_selection.entities.insert(entity);
    notifyChanged();
}

void SelectionManager::toggleSelect(EntityID entity) {
    auto it = m_selection.entities.find(entity);
    if (it != m_selection.entities.end()) {
        m_selection.entities.erase(it);
    } else {
        m_selection.entities.insert(entity);
    }
    notifyChanged();
}

void SelectionManager::addToSelection(EntityID entity) {
    m_selection.entities.insert(entity);
    notifyChanged();
}

void SelectionManager::removeFromSelection(EntityID entity) {
    m_selection.entities.erase(entity);
    // Also remove any sub-object selections for this entity
    m_selection.vertices.erase(entity);
    m_selection.edges.erase(entity);
    m_selection.faces.erase(entity);
    notifyChanged();
}

void SelectionManager::selectVertex(EntityID entity, uint32_t vertIdx) {
    m_selection.entities.insert(entity);
    m_selection.vertices[entity].insert(vertIdx);
    notifyChanged();
}

void SelectionManager::selectEdge(EntityID entity, uint32_t edgeIdx) {
    m_selection.entities.insert(entity);
    m_selection.edges[entity].insert(edgeIdx);
    notifyChanged();
}

void SelectionManager::selectFace(EntityID entity, uint32_t faceIdx) {
    m_selection.entities.insert(entity);
    m_selection.faces[entity].insert(faceIdx);
    notifyChanged();
}

void SelectionManager::toggleVertex(EntityID entity, uint32_t vertIdx) {
    auto& set = m_selection.vertices[entity];
    auto it = set.find(vertIdx);
    if (it != set.end()) {
        set.erase(it);
        if (set.empty()) {
            m_selection.vertices.erase(entity);
        }
    } else {
        set.insert(vertIdx);
        m_selection.entities.insert(entity);
    }
    notifyChanged();
}

void SelectionManager::toggleEdge(EntityID entity, uint32_t edgeIdx) {
    auto& set = m_selection.edges[entity];
    auto it = set.find(edgeIdx);
    if (it != set.end()) {
        set.erase(it);
        if (set.empty()) {
            m_selection.edges.erase(entity);
        }
    } else {
        set.insert(edgeIdx);
        m_selection.entities.insert(entity);
    }
    notifyChanged();
}

void SelectionManager::toggleFace(EntityID entity, uint32_t faceIdx) {
    auto& set = m_selection.faces[entity];
    auto it = set.find(faceIdx);
    if (it != set.end()) {
        set.erase(it);
        if (set.empty()) {
            m_selection.faces.erase(entity);
        }
    } else {
        set.insert(faceIdx);
        m_selection.entities.insert(entity);
    }
    notifyChanged();
}

void SelectionManager::clearSelection() {
    m_selection.clear();
    notifyChanged();
}

void SelectionManager::selectAll(Scene& /*scene*/) {
    // TODO: Iterate all entities in the scene and add them to the selection.
    // This requires Scene to expose an entity iteration API, which will be
    // implemented when the ECS is fully wired up.
    notifyChanged();
}

EntityID SelectionManager::primaryEntity() const {
    if (m_selection.entities.empty()) {
        return INVALID_ENTITY;
    }
    return *m_selection.entities.begin();
}

void SelectionManager::notifyChanged() {
    // TODO: Dispatch a SelectionChangedEvent through m_dispatcher when
    // the EventDispatcher system is implemented.
    (void)m_dispatcher;
}

glm::vec3 SelectionManager::selectionCenter(Scene& scene) const {
    if (m_selection.entities.empty()) {
        return glm::vec3(0.0f);
    }

    glm::vec3 center(0.0f);
    size_t count = 0;

    for (EntityID entityId : m_selection.entities) {
        Entity* entity = scene.getEntity(entityId);
        if (!entity) continue;

        auto* tc = entity->getComponent<TransformComponent>();
        if (!tc) continue;

        center += tc->position;
        count++;
    }

    if (count > 0) {
        return center / static_cast<float>(count);
    }

    return glm::vec3(0.0f);
}

} // namespace PixellentModeler
