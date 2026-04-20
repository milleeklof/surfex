import math as m

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


if __name__ == "__main__":
    plot1 = sx.init([-8.0, 8.0], [-8.0, 8.0])
    plot1.add(ripple, color="heatmap", alpha=1.0)

    plot2 = sx.init([-4.0, 4.0], [-4.0, 4.0])
    plot2.add(saddle, color="blue", alpha=1.0)

    plot3 = sx.init([-6.0, 6.0], [-6.0, 6.0])
    plot3.add(wave, color="green", alpha=1.0)

    sx.show()


# PYTHONPATH=python python examples/test.py
