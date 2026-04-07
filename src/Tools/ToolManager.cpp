#include "PixellentModeler/Tools/ToolManager.hpp"
#include "PixellentModeler/Tools/SelectTool.hpp"
#include "PixellentModeler/Tools/MoveTool.hpp"
#include "PixellentModeler/Tools/RotateTool.hpp"
#include "PixellentModeler/Tools/ScaleTool.hpp"
#include "PixellentModeler/Tools/ExtrudeTool.hpp"
#include <GLFW/glfw3.h>

namespace PixellentModeler {

ToolManager::ToolManager() = default;
ToolManager::~ToolManager() = default;

void ToolManager::init() {
    // Create all built-in tools
    m_tools.push_back(std::make_unique<SelectTool>());
    m_tools.push_back(std::make_unique<MoveTool>());
    m_tools.push_back(std::make_unique<RotateTool>());
    m_tools.push_back(std::make_unique<ScaleTool>());
    m_tools.push_back(std::make_unique<ExtrudeTool>());

    // Set the SelectTool as the default active tool
    if (!m_tools.empty()) {
        setActiveTool("Select");
    }
}

void ToolManager::setActiveTool(const std::string& name) {
    // Deactivate the current tool
    if (m_activeTool) {
        m_activeTool->setActive(false);
        m_activeTool->onDeactivate();
    }

    // Find the tool by name
    m_activeTool = nullptr;
    m_activeToolName.clear();
    for (auto& tool : m_tools) {
        if (tool->name() == name) {
            m_activeTool = tool.get();
            m_activeToolName = tool->name();
            m_activeTool->setActive(true);
            m_activeTool->onActivate();
            break;
        }
    }
}

const std::string& ToolManager::activeToolName() const {
    return m_activeToolName;
}

void ToolManager::onMouseDown(const ToolContext& ctx, int button, int mods) {
    if (m_activeTool) {
        m_activeTool->onMouseDown(ctx, button, mods);
    }
}

void ToolManager::onMouseMove(const ToolContext& ctx) {
    if (m_activeTool) {
        m_activeTool->onMouseMove(ctx);
    }
}

void ToolManager::onMouseUp(const ToolContext& ctx, int button, int mods) {
    if (m_activeTool) {
        m_activeTool->onMouseUp(ctx, button, mods);
    }
}

void ToolManager::onKeyDown(const ToolContext& ctx, int key, int mods) {
    // Check if the key matches a tool shortcut to switch tools
    for (auto& tool : m_tools) {
        const std::string& sc = tool->shortcut();
        if (!sc.empty() && sc.length() == 1) {
            // Map the shortcut letter to a GLFW key code (A=65, ...)
            int expectedKey = static_cast<int>(sc[0]);
            if (key == expectedKey) {
                setActiveTool(tool->name());
                return;
            }
        }
    }

    // Forward to active tool if no shortcut matched
    if (m_activeTool) {
        m_activeTool->onKeyDown(ctx, key, mods);
    }
}

void ToolManager::onRender(const ToolContext& ctx) {
    if (m_activeTool) {
        m_activeTool->onRender(ctx);
    }
}

} // namespace PixellentModeler
