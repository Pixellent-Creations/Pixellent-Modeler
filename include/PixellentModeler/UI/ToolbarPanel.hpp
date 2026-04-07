#pragma once

namespace PixellentModeler {

class ToolManager;

class ToolbarPanel {
public:
    void render(ToolManager* toolManager);

private:
    void toolButton(const char* label, const char* tooltip, bool active);
};

} // namespace PixellentModeler
