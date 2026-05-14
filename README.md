# Surfex

<p align="center">
  <a href="#overview">Overview</a> ·
  <a href="#screenshots">Screenshots</a> ·
  <a href="#usage">Usage</a> ·
  <a href="#controls">Controls</a> ·
  <a href="#clone-and-build">Clone and Build</a>
</p>

Surfex is a small, declarative surface explorer for plotting two-variable functions in Python.

It is a C++-accelerated Python library, built from first principles in OpenGL and C++, for quick visual inspection of mathematical surfaces, with support for multiple plots, optional heatmap coloring, and simple keyboard camera control.

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

Add more images here as the project evolves.


## Clone and Build

The library is made using CMake. To download, do

```bash
git clone ...
cd surfex

mkdir build
cd build

cmake ..
make
```

## Notes

- The Python package lives under `python/surfex`
- The native renderer lives in `src/` and `include/`
- The project uses GLFW, GLAD, GLM, and pybind11
