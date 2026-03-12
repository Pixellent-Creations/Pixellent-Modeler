#pragma once

#include <memory>

namespace PixellentModeler {

class Camera;

class Scene {
public:
    Scene();

    Camera& camera();

private:
    std::unique_ptr<Camera> m_camera;
};

} // namespace PixellentModeler
