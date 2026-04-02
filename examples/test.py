import surfex


def f(x, y):
    return 3.0 * x * y / (1.0 + x * x + y * y)


def g(x, y):
      return x*y



if __name__ == "__main__":
    surfex.init(g, [-5.0, 5.0], [-5.0, 5.0])
