# ImGui library
add_library(imgui STATIC
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_sdl2.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
)
target_link_libraries(imgui PUBLIC
    SDL2::SDL2
    OpenGL::GL
)

# ImPlot library
add_library(implot STATIC
    implot/implot.cpp
    implot/implot_items.cpp
)
target_include_directories(implot PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/implot
)
target_link_libraries(implot PUBLIC imgui)

# imgui-node-editor library
add_library(imgui_node_editor STATIC
    imgui-node-editor/imgui_node_editor.cpp
    imgui-node-editor/imgui_node_editor_api.cpp
    imgui-node-editor/imgui_canvas.cpp
    imgui-node-editor/crude_json.cpp
)
target_include_directories(imgui_node_editor PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui-node-editor
)
target_link_libraries(imgui_node_editor PUBLIC imgui)

# imgui-color-text-edit library
add_library(imgui_color_text_edit STATIC
    imgui-color-text-edit/text_editor.cpp
)
target_include_directories(imgui_color_text_edit PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui-color-text-edit
)
target_link_libraries(imgui_color_text_edit PUBLIC imgui)
