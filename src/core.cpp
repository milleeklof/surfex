#include <array>
#include <functional>

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "Surfex.h"

namespace py = pybind11;

namespace
{
void init_surface(const Surfex::Function2D& function,
                  const std::array<float, 2>& xRange,
                  const std::array<float, 2>& yRange)
{
    Surfex app(function, xRange, yRange);
    app.run();
}
}

PYBIND11_MODULE(_core, m)
{
    m.doc() = "Core bindings for Surfex";

    py::class_<Surfex>(m, "Surfex")
        .def(py::init<Surfex::Function2D,
                      std::array<float, 2>,
                      std::array<float, 2>>(),
             py::arg("function"),
             py::arg("x_range"),
             py::arg("y_range"))
        .def("set_resolution", &Surfex::setResolution,
             py::arg("nx"),
             py::arg("ny"))
        .def("set_window_size", &Surfex::setWindowSize,
             py::arg("width"),
             py::arg("height"))
        .def("set_title", &Surfex::setTitle,
             py::arg("title"))
        .def("run", &Surfex::run);

    m.def("init",
          &init_surface,
          py::arg("function"),
          py::arg("x_range"),
          py::arg("y_range"));
}
