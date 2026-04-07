#include "PixellentModeler/Scene/MeshComponent.hpp"
#include "PixellentModeler/Mesh/HalfEdgeMesh.hpp"
#include "PixellentModeler/Mesh/MeshBuffer.hpp"

namespace PixellentModeler {

void MeshComponent::rebuild() {
    if (!mesh || !buffer) {
        return;
    }

    // Re-triangulate the mesh and re-upload to GPU
    TriMeshData triData = mesh->triangulate();
    buffer->upload(triData);

    // Also rebuild wireframe data
    auto wireData = mesh->wireframeData();
    buffer->uploadWireframe(wireData);
}

} // namespace PixellentModeler
