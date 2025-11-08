cmake_minimum_required(VERSION 3.16)
set(PROJECT_NAME "dod")

project(${PROJECT_NAME})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

set(SDLTTF_VENDORED ON)
set(SDLTTF_PLUTOSVG OFF)
set(SDLTTF_HARFBUZZ OFF)

add_subdirectory(vendored/SDL3-3.2.26 EXCLUDE_FROM_ALL)
add_subdirectory(vendored/SDL3_ttf-3.1.0 EXCLUDE_FROM_ALL)
add_subdirectory(vendored/SDL3_image-3.2.4 EXCLUDE_FROM_ALL)

### init imgui
add_library(imgui
        vendored/imgui-1.92.4/imgui.cpp
        vendored/imgui-1.92.4/imgui_draw.cpp
        vendored/imgui-1.92.4/imgui_tables.cpp
        vendored/imgui-1.92.4/imgui_widgets.cpp
        vendored/imgui-1.92.4/imgui_demo.cpp
        vendored/imgui-1.92.4/backends/imgui_impl_sdl3.cpp
        vendored/imgui-1.92.4/backends/imgui_impl_sdlrenderer3.cpp
)
target_include_directories(imgui PUBLIC
        vendored/imgui-1.92.4
        vendored/imgui-1.92.4/backends
)
target_link_libraries(imgui PRIVATE SDL3::SDL3)
####################

add_executable(${PROJECT_NAME} WIN32
        main.cpp
        Game.cpp
        FloatingObject.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        SDL3_image::SDL3_image
        SDL3_ttf::SDL3_ttf
        SDL3::SDL3
        imgui
)
