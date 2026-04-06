import surfex
import math as m


def f(x, y):
    return 3.0 * x * y / (1.0 + x * x + y * y)


def g(x, y):
      return m.sin(x**2 + y**2) / (x**2 + y**2) 



if __name__ == "__main__":
    surfex.init(g, [-5.0, 5.0], [-5.0, 5.0])


# PYTHONPATH=python python examples/test.py
