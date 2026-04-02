from ._core import Surfex as _Surfex
from ._core import init as _init


class Surfex(_Surfex):
    """Thin Python-facing wrapper around the native Surfex class."""


def init(function, x_range, y_range):
    """Create and run a surface plot directly from Python."""
    return _init(function, x_range, y_range)


__all__ = ["Surfex", "init"]
