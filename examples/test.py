import math as m

import surfex as sx


def ripple(x, y):
    r = (100 * x * x + 100 * y * y) ** 0.5
    if r == 0.0:
        return 1.0
    return m.sin(r) / r

def twist(x, y):
    return 0.2 * m.sin(0.6 * x * y) + 0.05 * (x * x - y * y)

def saddle(x, y):
    return 0.35 * (x * x - y * y)


def wave(x, y):
    return 0.6 * m.sin(x) * m.cos(y)


if __name__ == "__main__":
    plot1 = sx.init([-8.0, 8.0], [-8.0, 8.0], 500)
    plot1.add(ripple, [-2.0, 8.0], [-2.0, 8.0], color="heatmap", alpha=1.0)
    plot1.add(twist, [-8.0, 8.0], [-8.0, 8.0], color="red", alpha=0.4)

    plot2 = sx.init([-4.0, 4.0], [-4.0, 4.0], 500)
    plot2.add(saddle, color="saddlebrown", alpha=1.0)

    plot3 = sx.init([-6.0, 6.0], [-6.0, 6.0], 500)
    plot3.add(wave, color="limegreen", alpha=1.0)

    sx.show()
