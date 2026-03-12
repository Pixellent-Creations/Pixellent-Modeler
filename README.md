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

Continue developing by implementing core systems (windowing, rendering, scene management, etc.). Keep the code original and modular to make sharing with the 3D modeling community easy.

