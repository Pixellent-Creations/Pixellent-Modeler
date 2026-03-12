# Pixellent-Modeler
A open source community made modeling program.

## 🛠️ Toolchain Setup
To build and develop Pixellent Modeler on Windows (or any platform supported by CMake), follow these steps:

1. **Install prerequisites**
   - [CMake](https://cmake.org/download/) (>= 3.10)
   - A C++17-capable compiler (MSVC, MinGW, Clang, GCC, etc.)
   - Optional dependencies such as GLFW, GLM, GLEW, etc. will be added as the project grows.

2. **Generate build files**
   Open a terminal in the repository root and run:
   ```sh
   cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
   ```

   The sample `main.cpp` constructs an `Application` object which in turn creates a `Window`, `Renderer`, and a minimal `Scene` containing a `Camera`. The renderer now:

1. Initializes GLAD and basic OpenGL state.
2. Creates a simple coloured triangle (VAO/VBO) and compiles a basic shader.
3. Each frame it computes a model‑view‑projection matrix from the scene's camera and draws the triangle.

The camera's aspect ratio is updated automatically when the window is resized, and the renderer adjusts its viewport accordingly. You can extend `Scene` with more objects, move the camera, or replace the triangle with real meshes.

3. **Compile**
   ```sh
   cmake --build build --config Debug
   ```

4. **Use VS Code tasks**
   The workspace includes `tasks.json` under `.vscode/` with two tasks:
   - **CMake: Configure** – runs the `cmake` configure step.
   - **CMake: Build** – builds the project (depends on configure).
   You can trigger them via the **Terminal ▶ Run Task…** menu or with `Ctrl+Shift+B`.

5. **Start coding**
   The `src/` and `include/` directories hold source files. Add new components as needed, and update `CMakeLists.txt` accordingly.

---

Continue developing by implementing core systems (windowing, rendering, scene management, etc.). The codebase now includes:

* `Window` – GLFW wrapper.
* `Renderer` – initializes GLAD, sets up a triangle VAO/VBO, and draws with a simple shader.
* `Shader` – utility for compiling/linking GLSL programs.
* `Scene` + `Camera` – basic scene container with a perspective camera.
* `Application` – high‑level class that ties everything together and drives the main loop.

Each of the first three milestones (GLAD loader, triangle rendering, camera/scene) has been implemented. Extend from here by adding mesh loading, input handling, and more elaborate scene graph features.

## 📦 Suggested Libraries
In the early stages you’ll want to pull in a few widely‑used dependencies. Here’s a starter set that covers most basic needs:

* **Window/input/context:** **GLFW** (used in examples) or SDL2
* **Math:** **GLM** (included example) or Eigen
* **Model import/export:** Assimp (later)
* **Rendering helpers:** **GLAD** (used below) or GLEW
* **GUI:** Dear ImGui (for tools/UI) or Qt (for full application)
* **Image IO:** stb_image / stb_image_write
* **Unit testing:** Catch2 or GoogleTest

You can add more as the project grows (CGAL for geometry, Python/Lua for scripting, Bullet for physics, etc.).

### Adding libraries with CMake
The top‑level `CMakeLists.txt` already compiles all `.cpp` files, but you’ll eventually need to link against dependencies. One convenient approach is `FetchContent`:

```cmake
include(FetchContent)

# Example: fetch GLFW and GLM automatically
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.3.8
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG        0.9.9.8
)
FetchContent_MakeAvailable(glm)

# Later, link libraries to your executable
# target_link_libraries(${PROJECT_NAME} PRIVATE glfw glm)
```

Adjust tags or use system packages/vcpkg as preferred.


