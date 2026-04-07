#pragma once

#include "PixellentModeler/Core/Command.hpp"
#include "PixellentModeler/Scene/TransformComponent.hpp"
#include <vector>
#include <glm/glm.hpp>

namespace PixellentModeler {

class Scene;

class TransformCommand : public Command {
public:
    struct EntityTransform {
        EntityID id;
        glm::vec3 oldPosition;
        glm::vec3 oldRotation;
        glm::vec3 oldScale;
        glm::vec3 newPosition;
        glm::vec3 newRotation;
        glm::vec3 newScale;
    };

    TransformCommand(Scene* scene, const std::vector<EntityTransform>& transforms);

    void execute() override;
    void undo() override;
    std::string description() const override { return "Transform"; }

private:
    Scene* m_scene;
    std::vector<EntityTransform> m_transforms;
};

} // namespace PixellentModeler
