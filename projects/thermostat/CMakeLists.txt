cmake_minimum_required(VERSION 3.15)
project(thermostat LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(SOURCES
    gctrl_udp_driver_windows.cpp
    thermostat.cpp
)
set(HEADERS
    gctrl.hpp
)
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/gctrl
)
add_executable(thermostat ${SOURCES} ${HEADERS})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set_target_properties(thermostat PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(thermostat PRIVATE
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Release>:-O2 -DNDEBUG>
    )
endif()

