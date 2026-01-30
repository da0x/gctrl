cmake_minimum_required(VERSION 3.16)
project(gctrl VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Platform detection
if(WIN32)
    set(GCTRL_PLATFORM_WINDOWS TRUE)
    add_compile_definitions(GCTRL_PLATFORM_WINDOWS)
elseif(UNIX AND NOT APPLE)
    set(GCTRL_PLATFORM_LINUX TRUE)
    add_compile_definitions(GCTRL_PLATFORM_LINUX)
endif()

# Find dependencies
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

# Add subdirectories
add_subdirectory(lib)
add_subdirectory(engine)
