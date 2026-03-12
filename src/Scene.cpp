#include "PixellentModeler/Scene.hpp"
#include "PixellentModeler/Camera.hpp"

namespace PixellentModeler {

Scene::Scene() {
    m_camera = std::make_unique<Camera>(45.0f, 4.0f/3.0f);
}

Camera& Scene::camera() {
    return *m_camera;
}

} // namespace PixellentModeler
