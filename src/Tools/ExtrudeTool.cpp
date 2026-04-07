#include "PixellentModeler/Tools/ExtrudeTool.hpp"
#include "PixellentModeler/Selection/SelectionManager.hpp"
#include "PixellentModeler/Selection/SelectionMode.hpp"
#include <GLFW/glfw3.h>
#include <cmath>

namespace PixellentModeler {

void ExtrudeTool::onMouseDown(const ToolContext& ctx, int button, int mods) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    if (!ctx.selection) return;

    // Extrude tool works only in Face selection mode
    if (ctx.selection->mode() != SelectionMode::Face) return;

    // Check that at least one face is selected
    const Selection& sel = ctx.selection->current();
    bool hasFaces = false;
    for (const auto& pair : sel.faces) {
        if (!pair.second.empty()) {
            hasFaces = true;
            break;
        }
    }
    if (!hasFaces) return;

    m_dragging = true;
    m_dragStart = ctx.mousePos;
    m_extrudeDistance = 0.0f;

    // TODO: Duplicate the selected faces and detach them from their neighbors,
    // creating new side faces connecting the original and duplicated faces.
    // This is the setup phase of the extrude operation.

    (void)mods;
}

void ExtrudeTool::onMouseMove(const ToolContext& ctx) {
    if (!m_dragging) return;

    // Compute extrude distance from vertical mouse drag
    float deltaY = -(ctx.mousePos.y - m_dragStart.y);
    m_extrudeDistance = deltaY * 0.01f; // scale factor for world units

    // TODO: Move the duplicated faces along their averaged normal by
    // m_extrudeDistance. Update the side face geometry accordingly.
    // This requires access to the mesh data of the selected entities.
}

void ExtrudeTool::onMouseUp(const ToolContext& ctx, int button, int mods) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;

    if (m_dragging) {
        if (std::abs(m_extrudeDistance) < 1e-4f) {
            // If the user didn't drag at all, cancel the extrude
            // TODO: Undo the face duplication setup
        } else {
            // TODO: Finalize the extrude by creating an ExtrudeCommand
            // and pushing it to ctx.commands for undo/redo support.
        }
    }

    m_dragging = false;
    m_extrudeDistance = 0.0f;
    (void)ctx;
    (void)mods;
}

} // namespace PixellentModeler
