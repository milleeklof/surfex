# Surfex

<p align="center">
  <a href="#overview">Overview</a> ·
  <a href="#requirements">Requirements</a> ·
  <a href="#architecture">Architecture</a> ·
  <a href="#screenshots">Screenshots</a> ·
  <a href="#usage">Usage</a> ·
  <a href="#controls">Controls</a> ·
  <a href="#clone-and-build">Clone and Build</a> ·
  <a href="#development">Development</a> ·
  <a href="#license">License</a>
</p>

Surfex is a small surface-plotting library for two-variable functions in Python.

It combines a Python-facing API with an OpenGL/C++ renderer for fast interactive inspection of mathematical surfaces.

<p align="center">
  <img src="screenshots/surfex_0001.png" width="32%" />
  <img src="screenshots/surfex_0002.png" width="32%" />
  <img src="screenshots/surfex_0003.png" width="32%" />
</p>

## Overview

- Plots functions of two variables as 3D surfaces
- Supports multiple windows shown in sequence
- Supports solid color and heatmap rendering
- Includes orbit-style camera controls
- Exposes a Python API for interactive use

## Requirements

- Python 3.10+
- CMake 3.20+
- A C++20 compiler
- OpenGL, GLFW, and libpng development packages
- `pybind11` in the same Python environment used for the build

## Architecture

- `src/` contains the native renderer and pybind11 bindings
- `include/` contains the native headers
- `python/surfex/` contains the installable Python package
- `python/surfex/shaders/` contains runtime shader assets installed alongside the package and loaded relative to `surfex._core`
- The compiled extension is installed as `surfex/_core*.so`

## Usage

```python
import surfex as sx
import math as m

def ripple(x, y):
    r = (x * x + y * y) ** 0.5
    if r == 0.0:
      return 1.0
    else:
      return m.sin(r)/r

def saddle(x, y):
    return 0.35 * (x * x - y * y)

plot1 = sx.init([-8.0, 8.0], [-8.0, 8.0])
plot1.add(ripple, color="heatmap", alpha=1.0)
plot1.add(saddle, color="blue", alpha=0.6)

plot2 = sx.init([-4.0, 4.0], [-4.0, 4.0])
plot2.add(saddle, color="red", alpha=1.0)

sx.show()
```

## Controls

- `H` / `Left Arrow`: rotate left
- `L` / `Right Arrow`: rotate right
- `J` / `Down Arrow`: tilt down
- `K` / `Up Arrow`: tilt up
- `W`: zoom in
- `S`: zoom out
- `X`, `Y`, `Z`: snap toward standard views with a short animation
- `P`: save a PNG screenshot to `screenshots/`
- `Q`: close the current window

## Screenshots

Add more screenshots here as the project evolves.


## Clone and Build

Surfex is installed from source with CMake. CMake uses the Python interpreter you pass in `Python_EXECUTABLE` to locate headers, pybind11, and the correct `site-packages` directory.

### macOS

```bash
brew install cmake glfw libpng pybind11
git clone ...
cd surfex

python3 -m venv .venv
source .venv/bin/activate
python -m pip install -U pip pybind11

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DPython_EXECUTABLE="$(which python)"
cmake --build build
cmake --install build
```

### Linux

Install `cmake`, `glfw`, and `libpng` with your package manager if they are not already present.

```bash
git clone ...
cd surfex

python3 -m venv .venv
source .venv/bin/activate
python -m pip install -U pip pybind11

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DPython_EXECUTABLE="$(which python)"
cmake --build build
cmake --install build
```

After installation, this should work without `PYTHONPATH`:

```python
import surfex
```

## Development

- Use `cmake -S . -B build -DPython_EXECUTABLE="$(which python)"` to configure against the active environment
- Use `cmake --build build` for incremental builds
- Use `cmake --install build` to install into the selected Python environment
- Optional developer toggles are `-DSURFEX_ENABLE_WARNINGS=ON` and `-DSURFEX_ENABLE_SANITIZERS=ON` with a Debug build
- The example script can be run after install with `python examples/example.py`

## License

BSD-3-Clause is the best default for a scientific/OpenGL library like this: permissive, widely used in research software, and compatible with closed or open downstream use.

MIT is simpler but slightly less explicit about endorsement/no-warranty language. GPL is stronger copyleft and reduces adoption for some downstream users.
