#pragma once

namespace PixellentModeler {

class Scene;
class SelectionManager;
class TransformComponent;
class MaterialComponent;
class LightComponent;

class PropertiesPanel {
public:
    void render(Scene& scene, SelectionManager* selection);

private:
    void drawTransformComponent(TransformComponent& transform);
    void drawMaterialComponent(MaterialComponent& material);
    void drawLightComponent(LightComponent& light);
};

} // namespace PixellentModeler
