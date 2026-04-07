#include "PixellentModeler/Core/MeshOperationCommand.hpp"
#include "PixellentModeler/Scene/Scene.hpp"
#include "PixellentModeler/Scene/Entity.hpp"
#include "PixellentModeler/Scene/MeshComponent.hpp"

namespace PixellentModeler {

MeshOperationCommand::MeshOperationCommand(Scene* scene, EntityID entityId,
                                           MeshOperationFunc operation,
                                           const std::string& description)
    : m_scene(scene), m_entityId(entityId),
      m_operation(operation), m_description(description) {
    // Snapshot the current mesh state
    Entity* entity = m_scene->getEntity(m_entityId);
    if (entity) {
        auto* meshComp = entity->getComponent<MeshComponent>();
        if (meshComp && meshComp->mesh) {
            m_meshSnapshot = *meshComp->mesh; // Copy current mesh state
        }
    }
}

void MeshOperationCommand::execute() {
    if (m_operationApplied) return; // Already executed

    Entity* entity = m_scene->getEntity(m_entityId);
    if (!entity) return;

    auto* meshComp = entity->getComponent<MeshComponent>();
    if (!meshComp || !meshComp->mesh) return;

    // Apply the operation
    if (m_operation(*meshComp->mesh)) {
        m_operationApplied = true;
        meshComp->mesh->recomputeNormals();
    }
}

void MeshOperationCommand::undo() {
    Entity* entity = m_scene->getEntity(m_entityId);
    if (!entity) return;

    auto* meshComp = entity->getComponent<MeshComponent>();
    if (!meshComp || !meshComp->mesh) return;

    // Restore from snapshot
    *meshComp->mesh = m_meshSnapshot;
    m_operationApplied = false;
}

} // namespace PixellentModeler
