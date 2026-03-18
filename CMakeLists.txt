cmake_minimum_required(VERSION 3.20)
project(function_plotter LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 REQUIRED)

add_executable(function-plotter
    src/main.cpp
    src/Mesh.cpp
    src/Camera.cpp
    src/glad.c
    src/Shader.cpp
    src/Axis.cpp
    src/Grid.cpp
)



target_include_directories(function-plotter PRIVATE
    include
)


target_link_libraries(function-plotter
    glfw
)
