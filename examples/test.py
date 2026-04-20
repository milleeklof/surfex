import math as m

import surfex as sx


def f(x, y):
    return 3.0 * x * y / (1.0 + x * x + y * y)


def g(x, y):
    return m.sin(x ** 2 + y ** 2) / (x ** 2 + y ** 2)


def h(x, y):
    return x + y


if __name__ == "__main__":
    plot1 = sx.init([-5.0, 5.0], [-5.0, 5.0])
    surf1 = plot1.add(f, color="blue", alpha=1.0)
    surf2 = plot1.add(g, color="heatmap", alpha=0.5)

    plot2 = sx.init([-5.0, 5.0], [-5.0, 5.0])
    surf3 = plot2.add(h, color="red", alpha=1.0)

    sx.show()


# PYTHONPATH=python python examples/test.py
