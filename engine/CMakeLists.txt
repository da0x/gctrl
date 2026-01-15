# Main application source
set(ENGINE_SOURCES
    main.cpp
)

# Platform-specific sources
if(GCTRL_PLATFORM_WINDOWS)
    list(APPEND ENGINE_SOURCES
        controls/network/udp_windows.cpp
        platform/windows/process.cpp
        platform/windows/dpi.cpp
        platform/windows/font.cpp
    )
elseif(GCTRL_PLATFORM_LINUX)
    list(APPEND ENGINE_SOURCES
        controls/network/udp_linux.cpp
        platform/linux/process.cpp
        platform/linux/dpi.cpp
        platform/linux/font.cpp
    )
endif()

add_executable(gctrl ${ENGINE_SOURCES})

target_include_directories(gctrl PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/nlohmann
    ${CMAKE_SOURCE_DIR}/lib/da0x
    ${CMAKE_SOURCE_DIR}/lib/IconFontCppHeaders
    ${CMAKE_SOURCE_DIR}/lib/L2DFileDialog/L2DFileDialog/src
)

target_link_libraries(gctrl PRIVATE
    imgui
    implot
    imgui_node_editor
    imgui_color_text_edit
    SDL2::SDL2
    OpenGL::GL
    GLEW::GLEW
)

# Platform-specific linking and settings
if(GCTRL_PLATFORM_WINDOWS)
    target_link_libraries(gctrl PRIVATE
        ws2_32
        dwmapi
        shcore
    )
    # Windows GUI application (no console)
    set_target_properties(gctrl PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
elseif(GCTRL_PLATFORM_LINUX)
    target_link_libraries(gctrl PRIVATE
        pthread
    )
endif()

# Copy font resources to build directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/fonts)
file(COPY
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/fonts/cascadia-code.ttf
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/fonts/consola.ttf
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/fonts/fa-solid-900.otf
    DESTINATION ${CMAKE_BINARY_DIR}/fonts
)
