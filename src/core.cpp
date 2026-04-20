#include <array>
#include <functional>
#include <string>

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "Surfex.h"

namespace py = pybind11;

PYBIND11_MODULE(_core, m)
{
    m.doc() = "Core bindings for Surfex";

    py::class_<Surfex::Surface>(m, "Surface")
        .def_readonly("color", &Surfex::Surface::color)
        .def_readonly("alpha", &Surfex::Surface::alpha);

    py::class_<Surfex>(m, "Surfex")
        .def(py::init<std::array<float, 2>, std::array<float, 2>>(),
             py::arg("x_range"),
             py::arg("y_range"))
        .def("add",
             py::overload_cast<Surfex::Function2D,
                               const std::string&,
                               float>(&Surfex::add),
             py::arg("function"),
             py::arg("color") = "blue",
             py::arg("alpha") = 1.0f)
        .def("add",
             py::overload_cast<Surfex::Function2D,
                               std::array<float, 2>,
                               std::array<float, 2>,
                               const std::string&,
                               float>(&Surfex::add),
             py::arg("function"),
             py::arg("x_range"),
             py::arg("y_range"),
             py::arg("color") = "blue",
             py::arg("alpha") = 1.0f)
        .def("set_resolution", &Surfex::setResolution,
             py::arg("nx"),
             py::arg("ny"))
        .def("set_window_size", &Surfex::setWindowSize,
             py::arg("width"),
             py::arg("height"))
        .def("set_title", &Surfex::setTitle,
             py::arg("title"))
        .def("run", &Surfex::run);
}
