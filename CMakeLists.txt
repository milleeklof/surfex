cmake_minimum_required(VERSION 3.20)
project(surfex LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ======================
# PYTHON + PYBIND11
# ======================
find_package(Python COMPONENTS Interpreter Development REQUIRED)

execute_process(
    COMMAND ${Python_EXECUTABLE} -m pybind11 --cmakedir
    OUTPUT_VARIABLE pybind11_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 REQUIRED)

# ======================
# GLFW
# ======================
find_package(glfw3 REQUIRED)
find_package(PNG REQUIRED)

# ======================
# CORE LIBRARY
# ======================
add_library(surfex_core
    src/Mesh.cpp
    src/Camera.cpp
    src/Shader.cpp
    src/Axis.cpp
    src/Grid.cpp
    src/Surfex.cpp
    src/glad.c
)

target_include_directories(surfex_core PUBLIC
    include
)

target_link_libraries(surfex_core
    glfw
    PNG::PNG
)

# ======================
# PYTHON MODULE
# ======================
pybind11_add_module(_core src/core.cpp)

target_link_libraries(_core PRIVATE surfex_core)

set_target_properties(_core PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/python/surfex
)

# ======================
# EXECUTABLE
# ======================
add_executable(surfex src/main.cpp)

target_link_libraries(surfex
    surfex_core
)
