# CMake script to generate templates.hpp at build time
# Usage: cmake -DSOURCE_DIR=<path> -DOUTPUT_FILE=<path> -P generate_templates.cmake

if(NOT DEFINED SOURCE_DIR)
    message(FATAL_ERROR "SOURCE_DIR not defined")
endif()

if(NOT DEFINED OUTPUT_FILE)
    message(FATAL_ERROR "OUTPUT_FILE not defined")
endif()

# Ensure output directory exists
get_filename_component(OUTPUT_DIR "${OUTPUT_FILE}" DIRECTORY)
file(MAKE_DIRECTORY "${OUTPUT_DIR}")

# Read template files
file(READ "${SOURCE_DIR}/gctrl.hpp" TEMPLATE_GCTRL_HPP)
file(READ "${SOURCE_DIR}/gctrl_udp_windows.cpp" TEMPLATE_GCTRL_UDP_WINDOWS)
file(READ "${SOURCE_DIR}/gctrl_udp_linux.cpp" TEMPLATE_GCTRL_UDP_LINUX)
file(READ "${SOURCE_DIR}/CMakeLists.txt.in" TEMPLATE_CMAKE_LISTS)

# Configure the template
configure_file(
    "${SOURCE_DIR}/templates.hpp.in"
    "${OUTPUT_FILE}"
    @ONLY
)
