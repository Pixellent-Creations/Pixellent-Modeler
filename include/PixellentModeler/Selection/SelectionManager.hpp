#pragma once

#include <unordered_set>
#include <unordered_map>
#include <cstdint>
#include "PixellentModeler/Selection/SelectionMode.hpp"
#include "PixellentModeler/Scene/Component.hpp"

namespace PixellentModeler {

class EventDispatcher;

struct Selection {
    SelectionMode mode = SelectionMode::Object;
    std::unordered_set<EntityID> entities;
    std::unordered_map<EntityID, std::unordered_set<uint32_t>> vertices;
    std::unordered_map<EntityID, std::unordered_set<uint32_t>> edges;
    std::unordered_map<EntityID, std::unordered_set<uint32_t>> faces;

    void clear();
    bool isEmpty() const;
    bool isEntitySelected(EntityID id) const;
};

class SelectionManager {
public:
    SelectionManager(EventDispatcher* dispatcher = nullptr);

    void setMode(SelectionMode mode);
    SelectionMode mode() const { return m_mode; }
    const Selection& current() const { return m_selection; }

    // Convenience accessors used by UI panels
    bool hasSelection() const { return !m_selection.isEmpty(); }
    EntityID selectedEntity() const { return primaryEntity(); }

    // Object selection
    void select(EntityID entity);
    void toggleSelect(EntityID entity);
    void addToSelection(EntityID entity);
    void removeFromSelection(EntityID entity);

    // Sub-object selection
    void selectVertex(EntityID entity, uint32_t vertIdx);
    void selectEdge(EntityID entity, uint32_t edgeIdx);
    void selectFace(EntityID entity, uint32_t faceIdx);
    void toggleVertex(EntityID entity, uint32_t vertIdx);
    void toggleEdge(EntityID entity, uint32_t edgeIdx);
    void toggleFace(EntityID entity, uint32_t faceIdx);

    void clearSelection();
    void selectAll(class Scene& scene);

    EntityID primaryEntity() const;

private:
    Selection m_selection;
    SelectionMode m_mode = SelectionMode::Object;
    EventDispatcher* m_dispatcher = nullptr;

    void notifyChanged();
};

} // namespace PixellentModeler
