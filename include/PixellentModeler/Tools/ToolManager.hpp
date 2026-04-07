#pragma once

#include <vector>
#include <memory>
#include <string>
#include "PixellentModeler/Tools/Tool.hpp"

namespace PixellentModeler {

class ToolManager {
public:
    ToolManager();
    ~ToolManager();

    void init();

    void setActiveTool(const std::string& name);
    Tool* activeTool() { return m_activeTool; }
    const std::string& activeToolName() const;

    void onMouseDown(const ToolContext& ctx, int button, int mods);
    void onMouseMove(const ToolContext& ctx);
    void onMouseUp(const ToolContext& ctx, int button, int mods);
    void onKeyDown(const ToolContext& ctx, int key, int mods);
    void onRender(const ToolContext& ctx);

    const std::vector<std::unique_ptr<Tool>>& tools() const { return m_tools; }

private:
    std::vector<std::unique_ptr<Tool>> m_tools;
    Tool* m_activeTool = nullptr;
    std::string m_activeToolName;
};

} // namespace PixellentModeler
