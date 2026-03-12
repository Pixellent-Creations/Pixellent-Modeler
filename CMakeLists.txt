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
# Example of using FetchContent to grab a couple of common libs:
include(FetchContent)

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

# Link libraries to the executable
target_link_libraries(${PROJECT_NAME} PRIVATE glfw glm)

# For GLFW we also need to define GLFW_INCLUDE_NONE to avoid downcasting
# and ensure linking to the proper system libraries on Windows
target_compile_definitions(${PROJECT_NAME} PRIVATE GLFW_INCLUDE_NONE)
