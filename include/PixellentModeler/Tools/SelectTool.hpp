#pragma once

#include "PixellentModeler/Tools/Tool.hpp"
#include <glm/glm.hpp>

namespace PixellentModeler {

class SelectTool : public Tool {
public:
    std::string name() const override { return "Select"; }
    std::string shortcut() const override { return "Q"; }

    void onMouseDown(const ToolContext& ctx, int button, int mods) override;
    void onMouseMove(const ToolContext& ctx) override;
    void onMouseUp(const ToolContext& ctx, int button, int mods) override;

private:
    bool m_dragging = false;
    glm::vec2 m_dragStart{0};
};

} // namespace PixellentModeler
