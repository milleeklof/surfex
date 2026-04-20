from ._core import Surfex as _Surfex
import gc


_plots = []


class Surfex(_Surfex):
    def __init__(self, x_range, y_range):
        super().__init__(x_range, y_range)
        _plots.append(self)


def init(x_range, y_range):
    return Surfex(x_range, y_range)


def show():
    plots = list(_plots)
    _plots.clear()

    for plot in plots:
        plot.run()
        del plot
        gc.collect()



__all__ = ["Surfex", "init", "show"]
