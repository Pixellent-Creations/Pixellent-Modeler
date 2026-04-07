#include "PixellentModeler/Scene/Entity.hpp"
#include <algorithm>

namespace PixellentModeler {

Entity::Entity(EntityID id, const std::string& name)
    : m_id(id), m_name(name) {
}

void Entity::setParent(Entity* parent) {
    // Remove from old parent's children list
    if (m_parent) {
        m_parent->removeChild(this);
    }

    m_parent = parent;

    // Add to new parent's children list
    if (m_parent) {
        m_parent->addChild(this);
    }
}

void Entity::addChild(Entity* child) {
    if (!child) {
        return;
    }

    // Avoid duplicates
    auto it = std::find(m_children.begin(), m_children.end(), child);
    if (it == m_children.end()) {
        m_children.push_back(child);
    }
}

void Entity::removeChild(Entity* child) {
    auto it = std::find(m_children.begin(), m_children.end(), child);
    if (it != m_children.end()) {
        m_children.erase(it);
    }
}

} // namespace PixellentModeler
