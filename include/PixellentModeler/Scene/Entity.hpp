#pragma once
#include "PixellentModeler/Scene/Component.hpp"
#include <typeindex>
#include <unordered_map>
#include <memory>
#include <vector>
#include <string>

namespace PixellentModeler {

class Entity {
public:
    Entity(EntityID id, const std::string& name = "Entity");

    EntityID id() const { return m_id; }
    const std::string& name() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }

    Entity* parent() const { return m_parent; }
    void setParent(Entity* parent);
    const std::vector<Entity*>& children() const { return m_children; }
    void addChild(Entity* child);
    void removeChild(Entity* child);

    template<typename T, typename... Args>
    T& addComponent(Args&&... args) {
        auto comp = std::make_unique<T>(std::forward<Args>(args)...);
        comp->owner = m_id;
        T& ref = *comp;
        m_components[std::type_index(typeid(T))] = std::move(comp);
        return ref;
    }

    template<typename T>
    T* getComponent() {
        auto it = m_components.find(std::type_index(typeid(T)));
        return it != m_components.end() ? static_cast<T*>(it->second.get()) : nullptr;
    }

    template<typename T>
    const T* getComponent() const {
        auto it = m_components.find(std::type_index(typeid(T)));
        return it != m_components.end() ? static_cast<const T*>(it->second.get()) : nullptr;
    }

    template<typename T>
    bool hasComponent() const {
        return m_components.count(std::type_index(typeid(T))) > 0;
    }

    template<typename T>
    void removeComponent() {
        m_components.erase(std::type_index(typeid(T)));
    }

private:
    EntityID m_id;
    std::string m_name;
    Entity* m_parent = nullptr;
    std::vector<Entity*> m_children;
    std::unordered_map<std::type_index, std::unique_ptr<Component>> m_components;
};

} // namespace PixellentModeler
