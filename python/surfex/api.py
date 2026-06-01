import inspect
import re

from ._core import Surfex as _Surfex
import gc


_plots = []


class Surfex(_Surfex):
    def __init__(self, x_range, y_range, subdivisions=500, title=None):
        super().__init__(x_range, y_range)
        self.set_resolution(subdivisions)
        _plots.append(self)
        inferred = title or _infer_plot_name() or "plot"
        if inferred.startswith("Surfex - "):
            self.set_title(inferred)
        else:
            self.set_title(f"Surfex - {inferred}")


def _infer_plot_name():
    frame = inspect.currentframe()
    try:
        caller = (
            frame.f_back.f_back.f_back
            if frame and frame.f_back and frame.f_back.f_back
            else None
        )
        if not caller:
            return None
        context = inspect.getframeinfo(caller).code_context or []
        if not context:
            return None
        match = re.match(r"^\s*([A-Za-z_][A-Za-z0-9_]*)\s*=\s*.*\.init\b", context[0])
        if match:
            return match.group(1)
    finally:
        del frame
    return None


def init(x_range, y_range, subdivisions=500, title=None):
    return Surfex(x_range, y_range, subdivisions, title)


def show():
    plots = list(_plots)
    _plots.clear()

    for plot in plots:
        plot.run()
        del plot
        gc.collect()



__all__ = ["Surfex", "init", "show"]
