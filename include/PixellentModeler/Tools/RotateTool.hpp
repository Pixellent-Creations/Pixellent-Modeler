#pragma once

#include "PixellentModeler/Tools/Tool.hpp"
#include "PixellentModeler/Tools/Gizmo.hpp"
#include <glm/glm.hpp>

namespace PixellentModeler {

class RotateTool : public Tool {
public:
    std::string name() const override { return "Rotate"; }
    std::string shortcut() const override { return "R"; }

    void onMouseDown(const ToolContext& ctx, int button, int mods) override;
    void onMouseMove(const ToolContext& ctx) override;
    void onMouseUp(const ToolContext& ctx, int button, int mods) override;
    void onRender(const ToolContext& ctx) override;

private:
    bool m_dragging = false;
    float m_totalAngle = 0.0f;
    glm::vec3 m_originalRotation{0};
    int m_activeAxis = -1;  // -1=none, 0=X, 1=Y, 2=Z
    bool m_snapping = false;
    float m_snapAngle = 15.0f;  // degrees
    Gizmo m_gizmo;
    glm::vec2 m_prevMousePos{0};
};

} // namespace PixellentModeler
