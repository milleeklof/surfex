cmake_minimum_required(VERSION 3.20)

project(surfex LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Generate compile_commands.json")

include(GNUInstallDirs)

option(SURFEX_ENABLE_WARNINGS "Enable common compiler warnings" ON)
option(SURFEX_ENABLE_SANITIZERS "Enable address/UB sanitizers in Debug builds" OFF)

# ----------------------------------------------------------------------------
# Python and pybind11
# ----------------------------------------------------------------------------
# The selected Python interpreter defines both the build headers and the
# installation target inside that interpreter's environment.
find_package(Python REQUIRED COMPONENTS Interpreter Development)

# Prefer pybind11 from the same Python environment as the chosen interpreter.
set(PYBIND11_FINDPYTHON ON)
if(NOT pybind11_DIR)
  execute_process(
    COMMAND "${Python_EXECUTABLE}" -m pybind11 --cmakedir
    OUTPUT_VARIABLE _surfex_pybind11_dir
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _surfex_pybind11_result
  )

  if(NOT _surfex_pybind11_result EQUAL 0 OR _surfex_pybind11_dir STREQUAL "")
    message(FATAL_ERROR
      "pybind11 was not found for Python interpreter: ${Python_EXECUTABLE}\n"
      "Install pybind11 into that environment or pass -Dpybind11_DIR=..."
    )
  endif()

  set(pybind11_DIR "${_surfex_pybind11_dir}")
  set(pybind11_DIR "${_surfex_pybind11_dir}" CACHE PATH "pybind11 CMake package directory")
endif()

find_package(pybind11 CONFIG REQUIRED)

# ----------------------------------------------------------------------------
# Native dependencies
# ----------------------------------------------------------------------------
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(PNG REQUIRED)

if(TARGET glfw)
  set(SURFEX_GLFW_TARGET glfw)
elseif(TARGET glfw3::glfw)
  set(SURFEX_GLFW_TARGET glfw3::glfw)
else()
  message(FATAL_ERROR "glfw3 package did not provide a known imported target")
endif()

# ----------------------------------------------------------------------------
# Python install path
# ----------------------------------------------------------------------------
# Install into the interpreter's platlib directory so the package lives beside
# its extension module in the correct environment.
execute_process(
  COMMAND "${Python_EXECUTABLE}" -c "import sysconfig; print(sysconfig.get_path('platlib') or sysconfig.get_path('purelib'))"
  OUTPUT_VARIABLE _surfex_python_platlib
  OUTPUT_STRIP_TRAILING_WHITESPACE
  RESULT_VARIABLE _surfex_python_platlib_result
)

if(NOT _surfex_python_platlib_result EQUAL 0 OR _surfex_python_platlib STREQUAL "")
  message(FATAL_ERROR
    "Failed to determine the Python site-packages directory for ${Python_EXECUTABLE}"
  )
endif()

set(SURFEX_PYTHON_PACKAGE_DIR "${_surfex_python_platlib}/surfex")
message(STATUS "Surfex Python package install dir: ${SURFEX_PYTHON_PACKAGE_DIR}")

function(surfex_apply_warnings target)
  if(SURFEX_ENABLE_WARNINGS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
      target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
  endif()

  if(SURFEX_ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE MATCHES "^[Dd]ebug$")
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
      target_compile_options(${target} PRIVATE -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer)
      target_link_options(${target} PRIVATE -fsanitize=address -fsanitize=undefined)
    endif()
  endif()
endfunction()

# ----------------------------------------------------------------------------
# Core native library
# ----------------------------------------------------------------------------
add_library(surfex_core STATIC
  src/Mesh.cpp
  src/Camera.cpp
  src/Shader.cpp
  src/Axis.cpp
  src/Grid.cpp
  src/Surfex.cpp
  src/glad.c
)

target_compile_features(surfex_core PUBLIC cxx_std_20)

set_target_properties(surfex_core PROPERTIES
  POSITION_INDEPENDENT_CODE ON
)

target_include_directories(surfex_core
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(surfex_core
  PUBLIC
    Python::Module
    pybind11::headers
    OpenGL::GL
    ${SURFEX_GLFW_TARGET}
    PNG::PNG
)

surfex_apply_warnings(surfex_core)

# ----------------------------------------------------------------------------
# Python extension module
# ----------------------------------------------------------------------------
pybind11_add_module(_core src/core.cpp)
target_compile_features(_core PRIVATE cxx_std_20)
target_link_libraries(_core PRIVATE surfex_core)
surfex_apply_warnings(_core)

# ----------------------------------------------------------------------------
# Installation
# ----------------------------------------------------------------------------
# Install the package as a regular package under site-packages:
#   surfex/__init__.py
#   surfex/api.py
#   surfex/_core.cpython-....so
install(TARGETS _core
  LIBRARY DESTINATION "${SURFEX_PYTHON_PACKAGE_DIR}"
)

install(FILES
  python/surfex/__init__.py
  python/surfex/api.py
  DESTINATION "${SURFEX_PYTHON_PACKAGE_DIR}"
)

install(DIRECTORY python/surfex/shaders
  DESTINATION "${SURFEX_PYTHON_PACKAGE_DIR}"
  FILES_MATCHING PATTERN "*.vert" PATTERN "*.frag" PATTERN "*.glsl"
)
