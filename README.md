# Surfex
Surfex is a small OpenGL surface explorer for plotting two-variable functions in Python.

It is built for quick visual inspection of mathematical surfaces, with support for multiple plots, optional heatmap coloring, and simple keyboard camera control.

<p align="center">
  <img src="images/sinc(r).png" width="45%" />
  <img src="images/cosrsinr.png" width="45%" />
</p>

## What It Does

- Plots functions of two variables as 3D surfaces
- Supports multiple windows shown in sequence
- Supports solid color and heatmap rendering
- Includes orbit-style camera controls
- Exposes a Python API for interactive use

## Usage

```python
import surfex as sx

def f(x, y):
    return x + y

plot = sx.init([-5.0, 5.0], [-5.0, 5.0])
plot.add(f, color="blue", alpha=1.0)
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
- `Q`: close the current window

## Screenshots

Add more images here as the project evolves.

<p align="center">
  <img src="images/sinc(r).png" width="45%" />
  <img src="images/cosrsinr.png" width="45%" />
</p>

## Clone and Build

_To be filled in._

## Notes

- The Python package lives under `python/surfex`
- The native renderer lives in `src/` and `include/`
- The project uses GLFW, GLAD, GLM, and pybind11
