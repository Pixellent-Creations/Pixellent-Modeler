#include "PixellentModeler/Core/Log.hpp"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace PixellentModeler {

// ANSI color codes
static constexpr const char* RESET   = "\033[0m";
static constexpr const char* GREEN   = "\033[32m";
static constexpr const char* YELLOW  = "\033[33m";
static constexpr const char* RED     = "\033[31m";
static constexpr const char* GRAY    = "\033[90m";

std::string Log::timestamp() {
    std::time_t now = std::time(nullptr);
    std::tm localTime{};
#ifdef _WIN32
    localtime_s(&localTime, &now);
#else
    localtime_r(&now, &localTime);
#endif
    std::ostringstream oss;
    oss << std::put_time(&localTime, "%H:%M:%S");
    return oss.str();
}

void Log::info(const std::string& message) {
    std::cout << GRAY << "[" << timestamp() << "] "
              << GREEN << "[INFO] " << RESET << message << std::endl;
}

void Log::warn(const std::string& message) {
    std::cout << GRAY << "[" << timestamp() << "] "
              << YELLOW << "[WARN] " << RESET << message << std::endl;
}

void Log::error(const std::string& message) {
    std::cerr << GRAY << "[" << timestamp() << "] "
              << RED << "[ERROR] " << RESET << message << std::endl;
}

} // namespace PixellentModeler
