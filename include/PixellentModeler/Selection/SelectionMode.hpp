#pragma once

#include <cstdint>

namespace PixellentModeler {

enum class SelectionMode : uint8_t {
    Object = 0,
    Vertex = 1,
    Edge = 2,
    Face = 3
};

} // namespace PixellentModeler
