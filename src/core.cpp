#include <array>
#include <functional>
#include <filesystem>
#include <string>

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "Surfex.h"

namespace py = pybind11;

namespace {

std::string functionNameFromPython(const py::function &function) {
    if (py::hasattr(function, "__name__")) {
        return py::cast<std::string>(function.attr("__name__"));
    }

    return "function";
}

} // namespace

PYBIND11_MODULE(_core, m)
{
    m.doc() = "Core bindings for Surfex";

    const std::filesystem::path packageDir =
        std::filesystem::path(py::cast<std::string>(m.attr("__file__")))
            .parent_path();
    m.attr("_package_dir") = packageDir.string();

    py::class_<Surfex::Surface>(m, "Surface")
        .def_readonly("color", &Surfex::Surface::color)
        .def_readonly("alpha", &Surfex::Surface::alpha);

    py::class_<Surfex>(m, "Surfex")
        .def(py::init<std::array<float, 2>, std::array<float, 2>>(),
             py::arg("x_range"),
             py::arg("y_range"))
        .def("set_title", &Surfex::setTitle, py::arg("title"))
        .def("add",
              [](Surfex &self, py::function function, const std::string &color, float alpha) {
                 const std::string functionName = functionNameFromPython(function);
                 Surfex::Function2D wrapped = [function](float x, float y) {
                     py::gil_scoped_acquire gil;
                     return py::cast<float>(function(x, y));
                 };
                 return self.addNamed(std::move(wrapped), functionName, color, alpha);
             },
             py::arg("function"),
             py::arg("color") = "blue",
             py::arg("alpha") = 1.0f)
        .def("add",
             [](Surfex &self, py::function function, std::array<float, 2> xRange,
                std::array<float, 2> yRange, const std::string &color, float alpha) {
                 const std::string functionName = functionNameFromPython(function);
                 Surfex::Function2D wrapped = [function](float x, float y) {
                     py::gil_scoped_acquire gil;
                     return py::cast<float>(function(x, y));
                 };
                 return self.addNamed(std::move(wrapped), xRange, yRange,
                                      functionName, color, alpha);
             },
             py::arg("function"),
             py::arg("x_range"),
             py::arg("y_range"),
             py::arg("color") = "blue",
             py::arg("alpha") = 1.0f)
         .def("set_resolution", &Surfex::setResolution,
              py::arg("n"))
        .def("set_window_size", &Surfex::setWindowSize,
             py::arg("width"),
             py::arg("height"))
         .def("run", &Surfex::run);
}
