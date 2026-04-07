#pragma once

#include <string>

namespace PixellentModeler {

class Scene;

class ProjectSerializer {
public:
    static bool save(const Scene& scene, const std::string& path);
    static bool load(Scene& scene, const std::string& path);
};

} // namespace PixellentModeler
