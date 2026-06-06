#pragma once

#include <cstddef>
#include <functional>
#include <vector>

inline constexpr int kMaxSurfaceResolution = 1024;

// A Mesh consists of vertices and indices
struct Mesh {
  std::vector<float> vertices;
  std::vector<unsigned int> indices;

  float minZ;
  float maxZ;
};

Mesh generateSurfaceMesh(
    std::function<float(float, float)> f,
    float xmin, float xmax,
    float ymin, float ymax,
    int n
);
