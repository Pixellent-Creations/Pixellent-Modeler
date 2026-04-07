#pragma once

#include "PixellentModeler/UI/ImGuiLayer.hpp"
#include "PixellentModeler/UI/ViewportPanel.hpp"
#include "PixellentModeler/UI/SceneHierarchyPanel.hpp"
#include "PixellentModeler/UI/PropertiesPanel.hpp"
#include "PixellentModeler/UI/ToolbarPanel.hpp"
#include "PixellentModeler/UI/MenuBar.hpp"
#include "PixellentModeler/UI/StatusBar.hpp"

struct GLFWwindow;

namespace PixellentModeler {

class Application;

class UIManager {
public:
    UIManager();
    ~UIManager();

    void init(GLFWwindow* window);
    void render(Application& app);
    void shutdown();

    ViewportPanel& viewport() { return m_viewport; }
    bool isViewportHovered() const { return m_viewport.isHovered(); }
    bool isViewportFocused() const { return m_viewport.isFocused(); }

private:
    ImGuiLayer           m_imguiLayer;
    ViewportPanel        m_viewport;
    SceneHierarchyPanel  m_hierarchy;
    PropertiesPanel      m_properties;
    ToolbarPanel         m_toolbar;
    MenuBar              m_menuBar;
    StatusBar            m_statusBar;

    bool m_firstFrame = true;
    void setupDockingLayout();
};

} // namespace PixellentModeler
