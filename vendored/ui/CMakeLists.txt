cmake_minimum_required(VERSION 3.31)
project(imgui LANGUAGES CXX)

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)

# Must set before add_library()/add_executable()
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

add_library(imgui STATIC
    imgui/imgui_demo.cpp
    imgui/imgui.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_draw.cpp
    imgui/backends/imgui_impl_sdl3.cpp
    imgui/backends/imgui_impl_sdlgpu3.cpp
    dear_bindings_generated/dcimgui.cpp
    dear_bindings_generated/backends/dcimgui_impl_sdl3.cpp
    dear_bindings_generated/backends/dcimgui_impl_sdlgpu3.cpp
    dear_bindings_generated/dcimgui_internal.cpp
)

# This replaces target_include_directories()
target_sources(imgui
    PUBLIC FILE_SET HEADERS
    BASE_DIRS
    imgui
    dear_bindings_generated
    FILES
    dear_bindings_generated/dcimgui_internal.h
    dear_bindings_generated/dcimgui.h
    PUBLIC FILE_SET imgui_backends TYPE HEADERS
    BASE_DIRS
    imgui/backends
    dear_bindings_generated/backends
    FILES
    dear_bindings_generated/backends/dcimgui_impl_sdl3.h
    dear_bindings_generated/backends/dcimgui_impl_sdlgpu3.h
)

target_link_libraries(imgui SDL3::SDL3)
