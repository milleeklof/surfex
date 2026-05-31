#include "Mesh.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include <glm/glm.hpp>

Mesh generateSurfaceMesh(std::function<float(float, float)> f, float xmin,
                         float xmax, float ymin, float ymax, int Nx, int Ny) {
  Mesh mesh;

  mesh.vertices.reserve(Nx * Ny * 6);
  mesh.indices.reserve((Nx - 1) * (Ny - 1) * 6);

  mesh.minZ = std::numeric_limits<float>::max();
  mesh.maxZ = -std::numeric_limits<float>::max();

  for (int i = 0; i < Nx; ++i) {
    for (int j = 0; j < Ny; ++j) {
      const float x = xmin + i * (xmax - xmin) / (Nx - 1);
      const float y = ymin + j * (ymax - ymin) / (Ny - 1);
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

  for (int j = 0; j < Ny - 1; ++j) {
    for (int i = 0; i < Nx - 1; ++i) {
      const int row1 = j * Nx;
      const int row2 = (j + 1) * Nx;

      mesh.indices.push_back(row1 + i);
      mesh.indices.push_back(row2 + i);
      mesh.indices.push_back(row1 + i + 1);

      mesh.indices.push_back(row1 + i + 1);
      mesh.indices.push_back(row2 + i);
      mesh.indices.push_back(row2 + i + 1);
    }
  }

  mesh.subdivisions = static_cast<std::size_t>(Nx) * static_cast<std::size_t>(Ny);
  mesh.maxDepth = 0;
  mesh.generationMs = 0.0;

  return mesh;
}
