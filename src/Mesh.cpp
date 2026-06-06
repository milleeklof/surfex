#include "Mesh.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>

#include <glm/glm.hpp>

namespace {

std::size_t checkedMultiply(std::size_t lhs, std::size_t rhs,
                            const char *what) {
  if (lhs != 0 && rhs > std::numeric_limits<std::size_t>::max() / lhs) {
    throw std::overflow_error(std::string("Surface mesh ") + what +
                              " count overflowed.");
  }
  return lhs * rhs;
}

void validateResolution(int n) {
  if (n < 2) {
    throw std::invalid_argument("Resolution must be between 2 and " +
                                std::to_string(kMaxSurfaceResolution) + ".");
  }
  if (n > kMaxSurfaceResolution) {
    throw std::invalid_argument("Resolution must be between 2 and " +
                                std::to_string(kMaxSurfaceResolution) + ".");
  }
}

} // namespace

Mesh generateSurfaceMesh(std::function<float(float, float)> f, float xmin,
                         float xmax, float ymin, float ymax, int n) {
  validateResolution(n);

  const std::size_t nPoints = static_cast<std::size_t>(n);
  Mesh mesh;

  mesh.vertices.reserve(checkedMultiply(checkedMultiply(nPoints, nPoints,
                                                        "vertex"),
                                        6, "vertex"));
  mesh.indices.reserve(checkedMultiply(checkedMultiply(nPoints - 1,
                                                        nPoints - 1,
                                                        "index"),
                                        6, "index"));

  mesh.minZ = std::numeric_limits<float>::max();
  mesh.maxZ = -std::numeric_limits<float>::max();

  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      const float x = xmin + i * (xmax - xmin) / (n - 1);
      const float y = ymin + j * (ymax - ymin) / (n - 1);
      const float z = f(x, y);

      mesh.minZ = std::min(mesh.minZ, z);
      mesh.maxZ = std::max(mesh.maxZ, z);

      const float h = 0.001f;
      const float fx = (f(x + h, y) - f(x - h, y)) / (2.0f * h);
      const float fy = (f(x, y + h) - f(x, y - h)) / (2.0f * h);

      glm::vec3 normal(-fx, -fy, 1.0f);
      if (glm::length(normal) != 0.0f) {
        normal = glm::normalize(normal);
      } else {
        normal = glm::vec3(0.0f, 0.0f, 1.0f);
      }

      mesh.vertices.push_back(x);
      mesh.vertices.push_back(y);
      mesh.vertices.push_back(z);
      mesh.vertices.push_back(normal.x);
      mesh.vertices.push_back(normal.y);
      mesh.vertices.push_back(normal.z);
    }
  }

  for (int j = 0; j < n - 1; ++j) {
    for (int i = 0; i < n - 1; ++i) {
      const int row1 = j * n;
      const int row2 = (j + 1) * n;

      mesh.indices.push_back(row1 + i);
      mesh.indices.push_back(row2 + i);
      mesh.indices.push_back(row1 + i + 1);

      mesh.indices.push_back(row1 + i + 1);
      mesh.indices.push_back(row2 + i);
      mesh.indices.push_back(row2 + i + 1);
    }
  }

  return mesh;
}
