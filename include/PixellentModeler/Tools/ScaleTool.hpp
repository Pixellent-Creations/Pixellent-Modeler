#pragma once

#include "PixellentModeler/Tools/Tool.hpp"
#include "PixellentModeler/Tools/Gizmo.hpp"
#include <glm/glm.hpp>

namespace PixellentModeler {

class ScaleTool : public Tool {
public:
    std::string name() const override { return "Scale"; }
    std::string shortcut() const override { return "S"; }

    void onMouseDown(const ToolContext& ctx, int button, int mods) override;
    void onMouseMove(const ToolContext& ctx) override;
    void onMouseUp(const ToolContext& ctx, int button, int mods) override;
    void onRender(const ToolContext& ctx) override;

private:
    bool m_dragging = false;
    glm::vec3 m_originalScale{1};
    float m_scaleFactor = 1.0f;
    int m_activeAxis = -1;  // -1=none, 0=X, 1=Y, 2=Z
    bool m_snapping = false;
    float m_snapSize = 0.1f;
    Gizmo m_gizmo;
    glm::vec2 m_prevMousePos{0};
};

} // namespace PixellentModeler
