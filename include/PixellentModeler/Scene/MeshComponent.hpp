#pragma once
#include "PixellentModeler/Scene/Component.hpp"
#include <memory>

namespace PixellentModeler {

class HalfEdgeMesh;
class MeshBuffer;

class MeshComponent : public Component {
public:
    std::shared_ptr<HalfEdgeMesh> mesh;
    std::shared_ptr<MeshBuffer> buffer;

    void rebuild();  // re-triangulate mesh and re-upload to GPU
};

} // namespace PixellentModeler
