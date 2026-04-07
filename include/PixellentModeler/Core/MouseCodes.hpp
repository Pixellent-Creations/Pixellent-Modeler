#pragma once

#include <GLFW/glfw3.h>

namespace PixellentModeler {

enum class MouseButton : int {
    Left    = GLFW_MOUSE_BUTTON_LEFT,
    Right   = GLFW_MOUSE_BUTTON_RIGHT,
    Middle  = GLFW_MOUSE_BUTTON_MIDDLE,
    Button4 = GLFW_MOUSE_BUTTON_4,
    Button5 = GLFW_MOUSE_BUTTON_5
};

} // namespace PixellentModeler
