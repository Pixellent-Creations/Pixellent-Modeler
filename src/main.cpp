#include <iostream>
#include "PixellentModeler/Window.hpp"
#include <glm/glm.hpp>

int main() {
    std::cout << "Pixellent Modeler starting..." << std::endl;

    PixellentModeler::Window window(640, 480, "Pixellent Test");

    // simple glm usage
    glm::vec3 v(1.0f, 2.0f, 3.0f);
    glm::mat4 m = glm::mat4(1.0f);
    glm::vec3 transformed = m * glm::vec4(v, 1.0f);
    std::cout << "glm transformed vector: (" << transformed.x << ", " << transformed.y << ", " << transformed.z << ")\n";

    while (!window.shouldClose()) {
        window.pollEvents();
        // rendering would go here
        window.swapBuffers();
    }

    return 0;
}
