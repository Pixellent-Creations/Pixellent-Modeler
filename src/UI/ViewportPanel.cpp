#include "PixellentModeler/UI/ViewportPanel.hpp"
#include "PixellentModeler/Rendering/Renderer.hpp"
#include "PixellentModeler/Scene/Scene.hpp"
#include "PixellentModeler/Scene/Camera.hpp"
#include "PixellentModeler/Rendering/Framebuffer.hpp"
#include "PixellentModeler/Scene/Entity.hpp"
#include "PixellentModeler/Scene/MeshComponent.hpp"
#include "PixellentModeler/Scene/TransformComponent.hpp"
#include "PixellentModeler/Scene/MaterialComponent.hpp"
#include "PixellentModeler/Scene/LightComponent.hpp"
#include "PixellentModeler/Mesh/MeshBuffer.hpp"

#include "imgui.h"
#include <glad/glad.h>

namespace PixellentModeler {

void ViewportPanel::init() {
    FramebufferSpec spec;
    spec.width  = static_cast<int>(m_viewportSize.x);
    spec.height = static_cast<int>(m_viewportSize.y);
    m_framebuffer = std::make_unique<Framebuffer>(spec);
}

void ViewportPanel::render(Renderer& renderer, Scene& scene) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport");

    // Track focus and hover state for input routing
    m_hovered = ImGui::IsWindowHovered();
    m_focused = ImGui::IsWindowFocused();

    // Get the content region size available for the viewport image
    ImVec2 availSize = ImGui::GetContentRegionAvail();
    glm::vec2 newSize{availSize.x, availSize.y};

    // Resize framebuffer if the viewport size has changed
    if (newSize.x > 0 && newSize.y > 0 &&
        (newSize.x != m_viewportSize.x || newSize.y != m_viewportSize.y))
    {
        m_viewportSize = newSize;
        m_framebuffer->resize(static_cast<int>(m_viewportSize.x),
                              static_cast<int>(m_viewportSize.y));
        scene.camera().setAspectRatio(m_viewportSize.x / m_viewportSize.y);
    }

    // Record the screen-space position of the viewport image
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    m_viewportPos = glm::vec2(cursorPos.x, cursorPos.y);

    // ---- Render the scene into the framebuffer ----
    m_framebuffer->bind();

    renderer.resize(static_cast<int>(m_viewportSize.x),
                    static_cast<int>(m_viewportSize.y));
    renderer.clear(0.1f, 0.1f, 0.1f, 1.0f);

    // Begin the renderer frame with camera matrices
    const Camera& cam = scene.camera();
    renderer.beginFrame(cam.view(), cam.projection());

    // Collect lights from the scene
    std::vector<LightData> lights;
    for (const auto& entity : scene.entities()) {
        if (entity->hasComponent<LightComponent>()) {
            auto* lc = entity->getComponent<LightComponent>();
            if (lc) {
                LightData ld = lc->toLightData();
                // If the entity also has a transform, use its position
                if (entity->hasComponent<TransformComponent>()) {
                    auto* tc = entity->getComponent<TransformComponent>();
                    if (tc) ld.position = tc->position;
                }
                lights.push_back(ld);
            }
        }
    }
    renderer.setLights(lights);

    // Draw the grid
    renderer.drawGrid();

    // Iterate entities and draw those with mesh components
    for (const auto& entity : scene.entities()) {
        if (!entity->hasComponent<MeshComponent>()) continue;

        auto* meshComp = entity->getComponent<MeshComponent>();
        if (!meshComp || !meshComp->mesh || !meshComp->buffer) continue;

        // Get model matrix from transform (or identity)
        glm::mat4 modelMatrix(1.0f);
        if (entity->hasComponent<TransformComponent>()) {
            auto* tc = entity->getComponent<TransformComponent>();
            if (tc) modelMatrix = tc->modelMatrix();
        }

        // Get material
        Material mat;
        if (entity->hasComponent<MaterialComponent>()) {
            auto* mc = entity->getComponent<MaterialComponent>();
            if (mc) mat = mc->toMaterial();
        }

        renderer.drawMesh(*meshComp->buffer, modelMatrix, mat);
    }

    renderer.endFrame();

    m_framebuffer->unbind();

    // Display the color attachment texture as an ImGui image
    // Flip the UVs vertically because OpenGL textures have (0,0) at bottom-left
    ImTextureID texID = reinterpret_cast<ImTextureID>(
        static_cast<uintptr_t>(m_framebuffer->colorAttachmentID(0)));
    ImGui::Image(texID,
                 ImVec2(m_viewportSize.x, m_viewportSize.y),
                 ImVec2(0, 1), ImVec2(1, 0));

    // ---- Viewport camera controls ----
    if (m_hovered) {
        ImGuiIO& io = ImGui::GetIO();

        // Scroll to zoom
        if (io.MouseWheel != 0.0f) {
            scene.camera().zoom(io.MouseWheel * 0.5f);
        }

        // Middle mouse button: orbit (or pan with Shift)
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
            ImVec2 delta = io.MouseDelta;
            if (io.KeyShift) {
                scene.camera().pan(-delta.x * 0.01f, delta.y * 0.01f);
            } else {
                scene.camera().orbit(delta.x * 0.3f, delta.y * 0.3f);
            }
        }

        // Right mouse button: also orbit (alternative)
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
            ImVec2 delta = io.MouseDelta;
            scene.camera().orbit(delta.x * 0.3f, delta.y * 0.3f);
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

} // namespace PixellentModeler
