#pragma once

struct GLFWwindow;

namespace PixellentModeler {

class ImGuiLayer {
public:
    ImGuiLayer() = default;
    ~ImGuiLayer();

    void init(GLFWwindow* window);
    void shutdown();
    void beginFrame();
    void endFrame();

    bool initialized() const { return m_initialized; }

private:
    bool m_initialized = false;
};

} // namespace PixellentModeler
