#include "Surfex.h"

#include <cmath>
#include <iostream>

namespace {
float func(float x, float y) { return 3.0f * std::cos(x) * std::sin(x); }
} // namespace

int main() {
  try {
    Surfex app({-15.0f, 15.0f}, {-15.0f, 15.0f});
    app.setResolution(1500, 1500);
    app.setTitle("Surfex (Surface Explorer)");
    app.add(func, "blue", 1.0f);
    app.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return -1;
  }

  return 0;
}
