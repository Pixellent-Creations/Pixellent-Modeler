#pragma once

#include <string>
#include <memory>
#include <vector>

namespace PixellentModeler {

class HalfEdgeMesh;
class Scene;

class ObjImporter {
public:
    struct ImportResult {
        bool success = false;
        std::string error;
        std::vector<std::shared_ptr<HalfEdgeMesh>> meshes;
        std::vector<std::string> names;
    };

    static ImportResult import(const std::string& path);
    static void importToScene(const std::string& path, Scene& scene);
};

} // namespace PixellentModeler
