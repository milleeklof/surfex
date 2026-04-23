import math as m
import numpy as np
import surfex as sx


def ripple(x, y):
    r = (x * x + y * y) ** 0.5
    if r == 0.0:
        return 1.0
    return m.sin(r) / r


def saddle(x, y):
    return 0.35 * (x * x - y * y)


def wave(x, y):
    return 0.6 * m.sin(x) * m.cos(y)

def f(x, y):
    L = 5.0
    n = 3
    m = 2
    return np.sin(n*np.pi*x/L) * np.sin(m*np.pi*y/L)

if __name__ == "__main__":
    plot1 = sx.init([-8.0, 8.0], [-8.0, 8.0])
    plot1.add(ripple, [0.0, 8.0], [0.0, 8.0], color="heatmap", alpha=1.0)

    plot2 = sx.init([-4.0, 4.0], [-4.0, 4.0])
    plot2.add(saddle, color="blue", alpha=1.0)

    plot3 = sx.init([-6.0, 6.0], [-6.0, 6.0])
    plot3.add(wave, color="green", alpha=1.0)

    plot4 = sx.init([0, 5], [0, 5])
    plot4.add(f)

    sx.show()


# PYTHONPATH=python python examples/test.py
