#pragma once

#include <string>

namespace PixellentModeler {

class HalfEdgeMesh;
class Scene;

class ObjExporter {
public:
    static bool exportMesh(const HalfEdgeMesh& mesh, const std::string& path);
    static bool exportScene(const Scene& scene, const std::string& path);
};

} // namespace PixellentModeler
