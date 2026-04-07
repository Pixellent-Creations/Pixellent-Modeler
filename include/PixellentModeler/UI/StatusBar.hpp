#pragma once

namespace PixellentModeler {

class Scene;
class SelectionManager;

class StatusBar {
public:
    void render(Scene* scene, SelectionManager* selection);

private:
    float m_fps = 0.0f;
    float m_frameTime = 0.0f;
    float m_fpsUpdateTimer = 0.0f;
    int   m_frameCount = 0;
};

} // namespace PixellentModeler
