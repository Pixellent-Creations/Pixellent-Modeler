#include "PixellentModeler/Tools/SelectTool.hpp"
#include "PixellentModeler/Selection/SelectionManager.hpp"
#include "PixellentModeler/Selection/Picker.hpp"
#include <GLFW/glfw3.h>

namespace PixellentModeler {

void SelectTool::onMouseDown(const ToolContext& ctx, int button, int mods) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;

    m_dragging = true;
    m_dragStart = ctx.mousePos;

    // Perform a pick at the current mouse position.
    // The Picker is expected to have already rendered the pick pass for this
    // frame. We access it through the renderer (which will be wired up once
    // the Renderer owns a Picker instance).
    // For now, we demonstrate the selection logic:

    if (!ctx.selection) return;

    // TODO: Obtain the picked entity from the Picker via the renderer.
    // EntityID picked = ctx.renderer->picker().pickEntity(
    //     static_cast<int>(ctx.mousePos.x),
    //     static_cast<int>(ctx.mousePos.y));
    EntityID picked = INVALID_ENTITY; // placeholder until Picker is wired

    if (picked == INVALID_ENTITY) {
        // Clicked on empty space -- clear selection unless shift is held
        if (!(mods & GLFW_MOD_SHIFT)) {
            ctx.selection->clearSelection();
        }
        return;
    }

    // Shift-click toggles selection; plain click replaces selection
    if (mods & GLFW_MOD_SHIFT) {
        ctx.selection->toggleSelect(picked);
    } else {
        ctx.selection->select(picked);
    }
}

void SelectTool::onMouseMove(const ToolContext& ctx) {
    if (!m_dragging) return;

    // Could implement marquee / box selection here in the future.
    // For now, track the drag for potential box select.
    (void)ctx;
}

void SelectTool::onMouseUp(const ToolContext& ctx, int button, int mods) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;

    if (m_dragging) {
        // If the mouse moved significantly, this was a box select drag
        glm::vec2 delta = ctx.mousePos - m_dragStart;
        float dragDist = glm::length(delta);

        if (dragDist > 5.0f) {
            // TODO: Implement box / marquee selection using the drag rectangle
            // defined by m_dragStart and ctx.mousePos.
        }
    }

    m_dragging = false;
    (void)mods;
}

} // namespace PixellentModeler
