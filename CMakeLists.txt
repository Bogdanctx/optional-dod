cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME "dod")
set(EXECUTABLE_NAME ${PROJECT_NAME})

project(${PROJECT_NAME})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
add_subdirectory(vendored/SDL3_ttf-3.2.2 EXCLUDE_FROM_ALL)

add_executable(${EXECUTABLE_NAME}
        main.cpp
        Game.cpp
        FloatingObject.cpp
)

target_link_libraries(${EXECUTABLE_NAME} PRIVATE SDL3_ttf::SDL3_ttf SDL3::SDL3)