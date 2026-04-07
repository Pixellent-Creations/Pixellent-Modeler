#pragma once

#include "PixellentModeler/Core/Command.hpp"
#include "PixellentModeler/Mesh/HalfEdgeMesh.hpp"
#include <functional>
#include <memory>

namespace PixellentModeler {

class Scene;

// Callback signature for mesh operation function
// Returns true on success, false on failure
using MeshOperationFunc = std::function<bool(HalfEdgeMesh&)>;

class MeshOperationCommand : public Command {
public:
    // Constructor stores a snapshot of the mesh before the operation
    MeshOperationCommand(Scene* scene, EntityID entityId,
                        MeshOperationFunc operation,
                        const std::string& description = "Mesh Operation");

    void execute() override;
    void undo() override;
    std::string description() const override { return m_description; }

private:
    Scene* m_scene;
    EntityID m_entityId;
    MeshOperationFunc m_operation;
    std::string m_description;
    HalfEdgeMesh m_meshSnapshot;  // Store mesh state before operation
    bool m_operationApplied = false;
};

} // namespace PixellentModeler
