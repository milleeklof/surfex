from ._core import Surfex as _Surfex
import gc


_plots = []


class Surfex(_Surfex):
    def __init__(self, x_range, y_range, subdivisions=500):
        super().__init__(x_range, y_range)
        self.set_resolution(subdivisions, subdivisions)
        _plots.append(self)


def init(x_range, y_range, subdivisions=500):
    return Surfex(x_range, y_range, subdivisions)


def show():
    plots = list(_plots)
    _plots.clear()

    for plot in plots:
        plot.run()
        del plot
        gc.collect()



__all__ = ["Surfex", "init", "show"]
