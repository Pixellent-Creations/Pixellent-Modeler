#pragma once

#include "PixellentModeler/Tools/Tool.hpp"
#include "PixellentModeler/Tools/Gizmo.hpp"
#include <glm/glm.hpp>

namespace PixellentModeler {

class MoveTool : public Tool {
public:
    std::string name() const override { return "Move"; }
    std::string shortcut() const override { return "W"; }

    void onMouseDown(const ToolContext& ctx, int button, int mods) override;
    void onMouseMove(const ToolContext& ctx) override;
    void onMouseUp(const ToolContext& ctx, int button, int mods) override;
    void onRender(const ToolContext& ctx) override;

private:
    bool m_dragging = false;
    glm::vec3 m_dragStart{0};
    glm::vec3 m_originalPosition{0};
    int m_activeAxis = -1;  // -1=none, 0=X, 1=Y, 2=Z
    bool m_snapping = false;
    float m_snapSize = 0.5f;
    Gizmo m_gizmo;
    glm::vec3 m_prevRayOrigin{0};
    glm::vec3 m_prevRayDir{0, 0, -1};
};

} // namespace PixellentModeler
