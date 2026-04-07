#include "PixellentModeler/UI/ImGuiLayer.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>
#include <iostream>

namespace PixellentModeler {

ImGuiLayer::~ImGuiLayer() {
    if (m_initialized) {
        shutdown();
    }
}

void ImGuiLayer::init(GLFWwindow* window) {
    if (m_initialized) {
        return;
    }

    // Verify ImGui version compatibility
    IMGUI_CHECKVERSION();

    // Create ImGui context
    ImGui::CreateContext();

    // Configure IO flags
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;    // Enable docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;  // Enable multi-viewport
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Keyboard navigation

    // When viewports are enabled, tweak WindowRounding/WindowBg so platform
    // windows can look identical to regular ones
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Set dark color theme
    ImGui::StyleColorsDark();

    // ---- Professional style customization ----
    style.WindowRounding    = 4.0f;
    style.FrameRounding     = 3.0f;
    style.GrabRounding      = 2.0f;
    style.TabRounding       = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.PopupRounding     = 3.0f;
    style.ChildRounding     = 3.0f;

    style.WindowPadding     = ImVec2(8.0f, 8.0f);
    style.FramePadding      = ImVec2(5.0f, 4.0f);
    style.ItemSpacing       = ImVec2(8.0f, 5.0f);
    style.ItemInnerSpacing  = ImVec2(6.0f, 4.0f);
    style.IndentSpacing     = 20.0f;
    style.ScrollbarSize     = 14.0f;
    style.GrabMinSize       = 10.0f;

    style.WindowBorderSize  = 1.0f;
    style.FrameBorderSize   = 0.0f;
    style.PopupBorderSize   = 1.0f;
    style.TabBorderSize     = 0.0f;

    // Subtle, professional dark palette
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg]           = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);
    colors[ImGuiCol_ChildBg]            = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);
    colors[ImGuiCol_PopupBg]            = ImVec4(0.14f, 0.14f, 0.15f, 0.96f);
    colors[ImGuiCol_Border]             = ImVec4(0.25f, 0.25f, 0.28f, 0.60f);
    colors[ImGuiCol_FrameBg]            = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]     = ImVec4(0.24f, 0.24f, 0.27f, 1.00f);
    colors[ImGuiCol_FrameBgActive]      = ImVec4(0.30f, 0.30f, 0.34f, 1.00f);
    colors[ImGuiCol_TitleBg]            = ImVec4(0.09f, 0.09f, 0.10f, 1.00f);
    colors[ImGuiCol_TitleBgActive]      = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]   = ImVec4(0.09f, 0.09f, 0.10f, 0.75f);
    colors[ImGuiCol_MenuBarBg]          = ImVec4(0.14f, 0.14f, 0.15f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]        = ImVec4(0.10f, 0.10f, 0.11f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]      = ImVec4(0.28f, 0.28f, 0.30f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.35f, 0.35f, 0.38f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.42f, 0.42f, 0.45f, 1.00f);
    colors[ImGuiCol_CheckMark]          = ImVec4(0.40f, 0.65f, 0.90f, 1.00f);
    colors[ImGuiCol_SliderGrab]         = ImVec4(0.40f, 0.65f, 0.90f, 0.80f);
    colors[ImGuiCol_SliderGrabActive]   = ImVec4(0.50f, 0.72f, 0.95f, 1.00f);
    colors[ImGuiCol_Button]             = ImVec4(0.22f, 0.22f, 0.25f, 1.00f);
    colors[ImGuiCol_ButtonHovered]      = ImVec4(0.30f, 0.30f, 0.34f, 1.00f);
    colors[ImGuiCol_ButtonActive]       = ImVec4(0.38f, 0.38f, 0.42f, 1.00f);
    colors[ImGuiCol_Header]             = ImVec4(0.22f, 0.22f, 0.25f, 1.00f);
    colors[ImGuiCol_HeaderHovered]      = ImVec4(0.30f, 0.30f, 0.34f, 1.00f);
    colors[ImGuiCol_HeaderActive]       = ImVec4(0.38f, 0.38f, 0.42f, 1.00f);
    colors[ImGuiCol_Separator]          = ImVec4(0.25f, 0.25f, 0.28f, 0.60f);
    colors[ImGuiCol_SeparatorHovered]   = ImVec4(0.40f, 0.65f, 0.90f, 0.78f);
    colors[ImGuiCol_SeparatorActive]    = ImVec4(0.40f, 0.65f, 0.90f, 1.00f);
    colors[ImGuiCol_Tab]               = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);
    colors[ImGuiCol_TabHovered]        = ImVec4(0.30f, 0.30f, 0.34f, 1.00f);
    colors[ImGuiCol_TabActive]         = ImVec4(0.22f, 0.22f, 0.25f, 1.00f);
    colors[ImGuiCol_TabUnfocused]      = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
    colors[ImGuiCol_DockingPreview]    = ImVec4(0.40f, 0.65f, 0.90f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg]    = ImVec4(0.10f, 0.10f, 0.11f, 1.00f);

    // Initialize platform/renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    m_initialized = true;
    std::cout << "ImGui layer initialized" << std::endl;
}

void ImGuiLayer::shutdown() {
    if (!m_initialized) {
        return;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    m_initialized = false;
    std::cout << "ImGui layer shut down" << std::endl;
}

void ImGuiLayer::beginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::endFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and render additional platform windows (multi-viewport)
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backupContext = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backupContext);
    }
}

} // namespace PixellentModeler
