cmake_minimum_required(VERSION 3.12)
project(imgui)

add_library(imgui STATIC)
add_library(imgui::imgui ALIAS imgui)

target_sources(
    imgui
    INTERFACE
    FILE_SET public_headers
    TYPE HEADERS
    BASE_DIRS include/
    FILES
        include/imgui/imconfig.h
        include/imgui/imgui.h
        include/imgui/imgui_impl_opengl3.h
        include/imgui/imgui_impl_sdl3.h

    PRIVATE
    FILE_SET private_headers
    TYPE HEADERS
    BASE_DIRS
        include/imgui
        src/
    FILES
        src/imgui_internal.h
        src/imstb_rectpack.h
        src/imstb_textedit.h
        src/imstb_truetype.h
)
target_sources(
    imgui
    PRIVATE
        src/imgui.cpp
        src/imgui_demo.cpp
        src/imgui_draw.cpp
        src/imgui_impl_opengl3.cpp
        src/imgui_impl_sdl3.cpp
        src/imgui_tables.cpp
        src/imgui_widgets.cpp
)
target_link_libraries(imgui PUBLIC SDL3::SDL3)
