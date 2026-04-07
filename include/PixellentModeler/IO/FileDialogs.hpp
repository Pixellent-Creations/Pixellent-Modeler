#pragma once

#include <string>

namespace PixellentModeler {

class FileDialogs {
public:
    // Returns empty string if cancelled
    static std::string openFile(const char* filter);
    static std::string saveFile(const char* filter);
};

} // namespace PixellentModeler
