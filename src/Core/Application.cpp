#include "PixellentModeler/Core/Application.hpp"
#include "PixellentModeler/Core/EventDispatcher.hpp"
#include "PixellentModeler/Core/Window.hpp"
#include "PixellentModeler/Core/InputManager.hpp"
#include "PixellentModeler/Core/CommandHistory.hpp"
#include "PixellentModeler/Core/Log.hpp"
#include "PixellentModeler/Core/Event.hpp"
#include "PixellentModeler/Scene/Scene.hpp"
#include "PixellentModeler/Scene/Entity.hpp"
#include "PixellentModeler/Scene/TransformComponent.hpp"
#include "PixellentModeler/Scene/MeshComponent.hpp"
#include "PixellentModeler/Scene/MaterialComponent.hpp"
#include "PixellentModeler/Scene/LightComponent.hpp"
#include "PixellentModeler/Rendering/Renderer.hpp"
#include "PixellentModeler/UI/ImGuiLayer.hpp"
#include "PixellentModeler/UI/UIManager.hpp"
#include "PixellentModeler/Selection/SelectionManager.hpp"
#include "PixellentModeler/Tools/ToolManager.hpp"
#include "PixellentModeler/Mesh/HalfEdgeMesh.hpp"
#include "PixellentModeler/Mesh/MeshPrimitives.hpp"
#include "PixellentModeler/Mesh/MeshBuffer.hpp"
#include "PixellentModeler/IO/ObjImporter.hpp"
#include "PixellentModeler/IO/ObjExporter.hpp"
#include "PixellentModeler/IO/ProjectSerializer.hpp"
#include "PixellentModeler/IO/FileDialogs.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

namespace PixellentModeler {

Application* Application::s_instance = nullptr;

Application::Application() {
    if (s_instance) {
        Log::error("Application instance already exists");
        return;
    }
    s_instance = this;

    Log::info("Initializing Pixellent Modeler");

    // Create core systems
    m_eventDispatcher = std::make_unique<EventDispatcher>();
    m_window = std::make_unique<Window>(1280, 720, "Pixellent Modeler");
    m_window->setEventDispatcher(m_eventDispatcher.get());
    m_inputManager = std::make_unique<InputManager>(*m_eventDispatcher);
    m_commandHistory = std::make_unique<CommandHistory>();

    // Create scene
    m_scene = std::make_unique<Scene>();

    // Create renderer and initialize with the window
    m_renderer = std::make_unique<Renderer>();
    m_renderer->init(m_window->native());

    // Create selection manager and tool manager
    m_selectionManager = std::make_unique<SelectionManager>();
    m_toolManager = std::make_unique<ToolManager>();
    m_toolManager->init();

    // Create UI manager (initializes ImGui internally)
    m_uiManager = std::make_unique<UIManager>();
    m_uiManager->init(m_window->native());

    // Subscribe to window close event
    m_eventDispatcher->subscribe(EventType::WindowClose, [this](Event& /*event*/) {
        m_running = false;
    });

    // Register Escape shortcut to close the application
    m_inputManager->registerShortcut("Close", GLFW_KEY_ESCAPE, 0, [this]() {
        m_running = false;
    });

    // Register Ctrl+Shift+Z for redo (must come before Ctrl+Z so the more
    // specific shortcut is matched first by the subset check)
    m_inputManager->registerShortcut("Redo", GLFW_KEY_Z, GLFW_MOD_CONTROL | GLFW_MOD_SHIFT, [this]() {
        m_commandHistory->redo();
    });

    // Register Ctrl+Z for undo
    m_inputManager->registerShortcut("Undo", GLFW_KEY_Z, GLFW_MOD_CONTROL, [this]() {
        m_commandHistory->undo();
    });

    Log::info("Application initialized successfully");

    // Add a default directional light so meshes are visible
    addDirectionalLight();
}

Application::~Application() {
    Log::info("Shutting down Pixellent Modeler");

    // Destroy in reverse order of creation
    m_toolManager.reset();
    m_selectionManager.reset();
    m_uiManager.reset();
    m_imguiLayer.reset();
    m_renderer.reset();
    m_scene.reset();
    m_commandHistory.reset();
    m_inputManager.reset();
    m_window.reset();
    m_eventDispatcher.reset();

    s_instance = nullptr;
}

void Application::run() {
    Log::info("Starting main loop");

    while (m_running && m_window && !m_window->shouldClose()) {
        // Begin frame
        m_inputManager->beginFrame();

        // Poll window events (triggers callbacks which dispatch events)
        m_window->pollEvents();

        // Render
        if (m_window->native()) {
            // Render the full UI (which internally renders the viewport scene)
            if (m_uiManager) {
                m_uiManager->render(*this);
            }
        }

        // Present
        m_window->swapBuffers();

        // End frame
        m_inputManager->endFrame();
    }

    Log::info("Main loop ended");
}

// ---------------------------------------------------------------------------
// Application actions (called by UIManager menu callbacks)
// ---------------------------------------------------------------------------

void Application::newScene() {
    if (m_scene) {
        m_scene->clear();
        Log::info("New scene created");
    }
}

void Application::openProject() {
    std::string path = FileDialogs::openFile("Pixellent Project (*.pxl)\0*.pxl\0All Files (*.*)\0*.*\0");
    if (path.empty()) return;

    if (m_scene) {
        ProjectSerializer::load(*m_scene, path);
        Log::info("Project loaded: " + path);
    }
}

void Application::saveProject() {
    // For now, always use Save As behavior
    saveProjectAs();
}

void Application::saveProjectAs() {
    std::string path = FileDialogs::saveFile("Pixellent Project (*.pxl)\0*.pxl\0All Files (*.*)\0*.*\0");
    if (path.empty()) return;

    if (m_scene) {
        ProjectSerializer::save(*m_scene, path);
        Log::info("Project saved: " + path);
    }
}

void Application::importObj() {
    std::string path = FileDialogs::openFile("OBJ Files (*.obj)\0*.obj\0All Files (*.*)\0*.*\0");
    if (path.empty()) return;

    if (m_scene) {
        ObjImporter::importToScene(path, *m_scene);
        Log::info("OBJ imported: " + path);
    }
}

void Application::exportObj() {
    std::string path = FileDialogs::saveFile("OBJ Files (*.obj)\0*.obj\0All Files (*.*)\0*.*\0");
    if (path.empty()) return;

    if (m_scene) {
        ObjExporter::exportScene(*m_scene, path);
        Log::info("OBJ exported: " + path);
    }
}

void Application::requestExit() {
    m_running = false;
}

void Application::undo() {
    if (m_commandHistory) {
        m_commandHistory->undo();
    }
}

void Application::redo() {
    if (m_commandHistory) {
        m_commandHistory->redo();
    }
}

void Application::deleteSelected() {
    if (!m_selectionManager || !m_scene) return;
    if (!m_selectionManager->hasSelection()) return;

    EntityID id = m_selectionManager->selectedEntity();
    m_scene->destroyEntity(id);
    m_selectionManager->clearSelection();
}

void Application::duplicateSelected() {
    if (!m_selectionManager || !m_scene) return;
    if (!m_selectionManager->hasSelection()) return;

    EntityID id = m_selectionManager->selectedEntity();
    m_scene->duplicateEntity(id);
}

void Application::selectAll() {
    // TODO: Implement select all entities
    Log::info("Select All (not yet implemented)");
}

void Application::addPrimitive(const std::string& name) {
    if (!m_scene) return;

    Entity& entity = m_scene->createEntity(name);

    auto& transform = entity.addComponent<TransformComponent>();
    transform.position = glm::vec3(0.0f);
    transform.rotation = glm::vec3(0.0f);
    transform.scale = glm::vec3(1.0f);

    auto& meshComp = entity.addComponent<MeshComponent>();

    // Generate the primitive mesh
    std::unique_ptr<HalfEdgeMesh> primMesh;
    if (name == "Cube") {
        primMesh = MeshPrimitives::createCube();
    } else if (name == "Sphere") {
        primMesh = MeshPrimitives::createUVSphere();
    } else if (name == "Cylinder") {
        primMesh = MeshPrimitives::createCylinder();
    } else if (name == "Cone") {
        primMesh = MeshPrimitives::createCone();
    } else if (name == "Plane") {
        primMesh = MeshPrimitives::createPlane();
    } else if (name == "Torus") {
        primMesh = MeshPrimitives::createTorus();
    }

    if (primMesh) {
        meshComp.mesh = std::move(primMesh);
    } else {
        meshComp.mesh = std::make_shared<HalfEdgeMesh>();
    }

    // Create GPU buffer and upload mesh data
    meshComp.buffer = std::make_shared<MeshBuffer>();
    meshComp.rebuild();

    auto& material = entity.addComponent<MaterialComponent>();
    material.albedo = glm::vec3(0.8f, 0.8f, 0.8f);
    material.metallic = 0.0f;
    material.roughness = 0.5f;

    Log::info("Added primitive: " + name);
}

void Application::addPointLight() {
    if (!m_scene) return;

    Entity& entity = m_scene->createEntity("Point Light");

    auto& transform = entity.addComponent<TransformComponent>();
    transform.position = glm::vec3(0.0f, 5.0f, 0.0f);

    auto& light = entity.addComponent<LightComponent>();
    light.type = LightType::Point;
    light.color = glm::vec3(1.0f);
    light.intensity = 1.0f;

    Log::info("Added point light");
}

void Application::addDirectionalLight() {
    if (!m_scene) return;

    Entity& entity = m_scene->createEntity("Directional Light");

    auto& transform = entity.addComponent<TransformComponent>();
    transform.rotation = glm::vec3(-45.0f, 0.0f, 0.0f);

    auto& light = entity.addComponent<LightComponent>();
    light.type = LightType::Directional;
    light.direction = glm::vec3(-0.3f, -1.0f, -0.5f);
    light.color = glm::vec3(1.0f);
    light.intensity = 1.0f;

    Log::info("Added directional light");
}

void Application::toggleWireframe() {
    if (m_renderer) {
        m_renderer->setWireframeEnabled(!m_renderer->wireframeEnabled());
    }
}

void Application::toggleGrid() {
    if (m_renderer) {
        m_renderer->setGridEnabled(!m_renderer->gridEnabled());
    }
}

void Application::resetCamera() {
    if (m_scene) {
        m_scene->camera().resetView();
    }
}

} // namespace PixellentModeler
