#include "PixellentModeler/Core.hpp"
#include <iostream>

namespace PixellentModeler {

class Application {
public:
    Application() {
        std::cout << "Application created" << std::endl;
    }
    void run() {
        std::cout << "Running main loop (not implemented)" << std::endl;
        // placeholder for main loop logic
    }
};

} // namespace PixellentModeler
