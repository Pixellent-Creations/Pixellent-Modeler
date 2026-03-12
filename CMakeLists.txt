cmake_minimum_required(VERSION 3.10)
project(PixellentModeler VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
file(GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# For now we don't link external libraries; they can be added later
# target_link_libraries(${PROJECT_NAME} PRIVATE glfw glm glew ...)
