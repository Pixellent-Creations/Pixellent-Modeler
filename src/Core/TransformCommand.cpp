#include "PixellentModeler/Core/TransformCommand.hpp"
#include "PixellentModeler/Scene/Scene.hpp"
#include "PixellentModeler/Scene/Entity.hpp"
#include "PixellentModeler/Scene/TransformComponent.hpp"

namespace PixellentModeler {

TransformCommand::TransformCommand(Scene* scene, const std::vector<EntityTransform>& transforms)
    : m_scene(scene), m_transforms(transforms) {
}

void TransformCommand::execute() {
    for (const auto& transform : m_transforms) {
        Entity* entity = m_scene->getEntity(transform.id);
        if (!entity) continue;

        auto* tc = entity->getComponent<TransformComponent>();
        if (!tc) continue;

        tc->position = transform.newPosition;
        tc->rotation = transform.newRotation;
        tc->scale = transform.newScale;
    }
}

void TransformCommand::undo() {
    for (const auto& transform : m_transforms) {
        Entity* entity = m_scene->getEntity(transform.id);
        if (!entity) continue;

        auto* tc = entity->getComponent<TransformComponent>();
        if (!tc) continue;

        tc->position = transform.oldPosition;
        tc->rotation = transform.oldRotation;
        tc->scale = transform.oldScale;
    }
}

} // namespace PixellentModeler
