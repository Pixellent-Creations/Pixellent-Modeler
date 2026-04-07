#pragma once

#include <string>

namespace PixellentModeler {

class Log {
public:
    static void info(const std::string& message);
    static void warn(const std::string& message);
    static void error(const std::string& message);

private:
    static std::string timestamp();
};

} // namespace PixellentModeler
