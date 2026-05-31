#include "Mesh.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

namespace {

constexpr float kAbsoluteErrorTolerance = 0.0005f;
constexpr float kRelativeErrorTolerance = 0.01f;
constexpr int kMaxDepthCap = 12;

struct GridPoint {
  std::uint64_t x = 0;
  std::uint64_t y = 0;

  bool operator==(const GridPoint &other) const {
    return x == other.x && y == other.y;
  }
};

struct GridPointHash {
  std::size_t operator()(const GridPoint &p) const noexcept {
    const std::size_t hx = std::hash<std::uint64_t>{}(p.x);
    const std::size_t hy = std::hash<std::uint64_t>{}(p.y);
    return hx ^ (hy + 0x9e3779b97f4a7c15ULL + (hx << 6) + (hx >> 2));
  }
};

struct Cell {
  std::uint64_t gx = 0;
  std::uint64_t gy = 0;
  std::uint64_t size = 0;
  int level = 0;
};

struct SampleCache {
  std::function<float(float, float)> f;
  float xmin = 0.0f;
  float xmax = 0.0f;
  float ymin = 0.0f;
  float ymax = 0.0f;
  std::uint64_t gridScale = 1;
  double xSpan = 1.0;
  double ySpan = 1.0;
  float minZ = std::numeric_limits<float>::max();
  float maxZ = -std::numeric_limits<float>::max();
  std::unordered_map<GridPoint, float, GridPointHash> values;
  std::unordered_map<GridPoint, glm::vec3, GridPointHash> normals;

  float toX(std::uint64_t gx) const {
    return static_cast<float>(static_cast<double>(xmin) +
                              (static_cast<double>(gx) /
                               static_cast<double>(gridScale)) * xSpan);
  }

  float toY(std::uint64_t gy) const {
    return static_cast<float>(static_cast<double>(ymin) +
                              (static_cast<double>(gy) /
                               static_cast<double>(gridScale)) * ySpan);
  }

  float valueAt(const GridPoint &p) {
    const auto it = values.find(p);
    if (it != values.end()) {
      return it->second;
    }

    const float x = toX(p.x);
    const float y = toY(p.y);
    const float z = f(x, y);
    values.emplace(p, z);
    minZ = std::min(minZ, z);
    maxZ = std::max(maxZ, z);
    return z;
  }

  glm::vec3 normalAt(const GridPoint &p) {
    const auto it = normals.find(p);
    if (it != normals.end()) {
      return it->second;
    }

    const float x = toX(p.x);
    const float y = toY(p.y);
    const float h = 0.001f;
    const float fx = (f(x + h, y) - f(x - h, y)) / (2.0f * h);
    const float fy = (f(x, y + h) - f(x, y - h)) / (2.0f * h);

    glm::vec3 normal(-fx, -fy, 1.0f);
    if (glm::length(normal) == 0.0f) {
      normal = glm::vec3(0.0f, 0.0f, 1.0f);
    } else {
      normal = glm::normalize(normal);
    }

    normals.emplace(p, normal);
    return normal;
  }
};

bool overlaps(std::uint64_t a0, std::uint64_t a1, std::uint64_t b0,
              std::uint64_t b1) {
  return std::max(a0, b0) < std::min(a1, b1);
}

bool touchesLeft(const Cell &a, const Cell &b) {
  return a.gx == b.gx + b.size && overlaps(a.gy, a.gy + a.size, b.gy,
                                           b.gy + b.size);
}

bool touchesRight(const Cell &a, const Cell &b) {
  return a.gx + a.size == b.gx && overlaps(a.gy, a.gy + a.size, b.gy,
                                           b.gy + b.size);
}

bool touchesBottom(const Cell &a, const Cell &b) {
  return a.gy == b.gy + b.size && overlaps(a.gx, a.gx + a.size, b.gx,
                                            b.gx + b.size);
}

bool touchesTop(const Cell &a, const Cell &b) {
  return a.gy + a.size == b.gy && overlaps(a.gx, a.gx + a.size, b.gx,
                                            b.gx + b.size);
}

float localErrorThreshold(float z00, float z10, float z01, float z11) {
  const float scale = std::max({std::fabs(z00), std::fabs(z10), std::fabs(z01),
                                std::fabs(z11), 1.0f});
  return std::max(kAbsoluteErrorTolerance, kRelativeErrorTolerance * scale);
}

float sampleDirect(SampleCache &cache, float x, float y) {
  const float z = cache.f(x, y);
  cache.minZ = std::min(cache.minZ, z);
  cache.maxZ = std::max(cache.maxZ, z);
  return z;
}

float cellRefinementError(const Cell &cell, SampleCache &cache) {
  const float x0 = cache.toX(cell.gx);
  const float x1 = cache.toX(cell.gx + cell.size);
  const float y0 = cache.toY(cell.gy);
  const float y1 = cache.toY(cell.gy + cell.size);

  const auto bilinear = [&](float u, float v, float z00, float z10, float z01,
                            float z11) {
    const float a = (1.0f - u) * (1.0f - v) * z00;
    const float b = u * (1.0f - v) * z10;
    const float c = (1.0f - u) * v * z01;
    const float d = u * v * z11;
    return a + b + c + d;
  };

  const auto probe = [&](float u, float v, float z00, float z10, float z01,
                         float z11) {
    const float x = x0 + (x1 - x0) * u;
    const float y = y0 + (y1 - y0) * v;
    const float actual = sampleDirect(cache, x, y);
    const float predicted = bilinear(u, v, z00, z10, z01, z11);
    return std::fabs(actual - predicted);
  };

  const float z00 = cache.valueAt({cell.gx, cell.gy});
  const float z10 = cache.valueAt({cell.gx + cell.size, cell.gy});
  const float z01 = cache.valueAt({cell.gx, cell.gy + cell.size});
  const float z11 = cache.valueAt({cell.gx + cell.size, cell.gy + cell.size});

  const float threshold = localErrorThreshold(z00, z10, z01, z11);
  const float maxError = std::max(
      {probe(0.50f, 0.50f, z00, z10, z01, z11),
       probe(0.25f, 0.25f, z00, z10, z01, z11),
       probe(0.75f, 0.25f, z00, z10, z01, z11),
       probe(0.25f, 0.75f, z00, z10, z01, z11),
       probe(0.75f, 0.75f, z00, z10, z01, z11),
       probe(0.21f, 0.37f, z00, z10, z01, z11),
       probe(0.37f, 0.79f, z00, z10, z01, z11),
       probe(0.61f, 0.23f, z00, z10, z01, z11),
       probe(0.79f, 0.58f, z00, z10, z01, z11)});

  return maxError - threshold;
}

std::vector<Cell> subdivideCell(const Cell &cell) {
  const std::uint64_t half = cell.size / 2;
  return {
      {cell.gx, cell.gy, half, cell.level + 1},
      {cell.gx + half, cell.gy, half, cell.level + 1},
      {cell.gx, cell.gy + half, half, cell.level + 1},
      {cell.gx + half, cell.gy + half, half, cell.level + 1}};
}

int maxDepthHint(int nx, int ny) {
  const int hinted = static_cast<int>(std::ceil(
      std::log2(static_cast<double>(std::max(std::max(nx, ny), 2)))));
  return std::clamp(hinted, 1, kMaxDepthCap);
}

unsigned int vertexIndexFor(const GridPoint &point, SampleCache &cache,
                            Mesh &mesh,
                            std::unordered_map<GridPoint, unsigned int,
                                               GridPointHash> &vertexMap) {
  const auto it = vertexMap.find(point);
  if (it != vertexMap.end()) {
    return it->second;
  }

  const unsigned int index = static_cast<unsigned int>(mesh.vertices.size() / 6);
  const float x = cache.toX(point.x);
  const float y = cache.toY(point.y);
  const float z = cache.valueAt(point);
  const glm::vec3 normal = cache.normalAt(point);

  mesh.vertices.push_back(x);
  mesh.vertices.push_back(y);
  mesh.vertices.push_back(z);
  mesh.vertices.push_back(normal.x);
  mesh.vertices.push_back(normal.y);
  mesh.vertices.push_back(normal.z);

  vertexMap.emplace(point, index);
  return index;
}

} // namespace

Mesh generateSurfaceMesh(std::function<float(float, float)> f, float xmin,
                         float xmax, float ymin, float ymax, int Nx, int Ny) {
  const auto totalStart = std::chrono::steady_clock::now();

  Mesh mesh;
  mesh.minZ = std::numeric_limits<float>::max();
  mesh.maxZ = -std::numeric_limits<float>::max();

  SampleCache cache;
  cache.f = std::move(f);
  cache.xmin = xmin;
  cache.xmax = xmax;
  cache.ymin = ymin;
  cache.ymax = ymax;
  cache.xSpan = static_cast<double>(xmax) - static_cast<double>(xmin);
  cache.ySpan = static_cast<double>(ymax) - static_cast<double>(ymin);
  if (cache.xSpan == 0.0) {
    cache.xSpan = 1.0;
  }
  if (cache.ySpan == 0.0) {
    cache.ySpan = 1.0;
  }

  const int maxDepth = maxDepthHint(Nx, Ny);
  cache.gridScale = 1ULL << maxDepth;

  std::vector<Cell> leaves;
  leaves.push_back({0, 0, cache.gridScale, 0});

  bool changed = false;
  int passCount = 0;
  do {
    changed = false;
    ++passCount;

    std::vector<Cell> refined;
    refined.reserve(leaves.size() * 4);

    for (const Cell &cell : leaves) {
      if (cell.level < maxDepth && cellRefinementError(cell, cache) > 0.0f) {
        const std::vector<Cell> children = subdivideCell(cell);
        refined.insert(refined.end(), children.begin(), children.end());
        changed = true;
      } else {
        refined.push_back(cell);
      }
    }

    leaves.swap(refined);

    std::vector<bool> balanceSplit(leaves.size(), false);
    for (std::size_t i = 0; i < leaves.size(); ++i) {
      const Cell &a = leaves[i];
      if (a.level >= maxDepth) {
        continue;
      }

      for (std::size_t j = 0; j < leaves.size(); ++j) {
        if (i == j) {
          continue;
        }

        const Cell &b = leaves[j];
        if ((touchesLeft(a, b) || touchesRight(a, b) || touchesBottom(a, b) ||
             touchesTop(a, b)) &&
            b.level > a.level + 1) {
          balanceSplit[i] = true;
          break;
        }
      }
    }

    if (std::any_of(balanceSplit.begin(), balanceSplit.end(),
                    [](bool value) { return value; })) {
      std::vector<Cell> balanced;
      balanced.reserve(leaves.size() * 4);

      for (std::size_t i = 0; i < leaves.size(); ++i) {
        const Cell &cell = leaves[i];
        if (balanceSplit[i] && cell.level < maxDepth) {
          const std::vector<Cell> children = subdivideCell(cell);
          balanced.insert(balanced.end(), children.begin(), children.end());
          changed = true;
        } else {
          balanced.push_back(cell);
        }
      }

      leaves.swap(balanced);
    }
  } while (changed && passCount < 64);

  std::unordered_map<GridPoint, unsigned int, GridPointHash> vertexMap;
  mesh.vertices.reserve(leaves.size() * 6);
  mesh.indices.reserve(leaves.size() * 8);

  for (const Cell &cell : leaves) {
    const bool needLeft = std::any_of(leaves.begin(), leaves.end(), [&](const Cell &other) {
      return touchesLeft(cell, other) && other.level > cell.level;
    });

    const bool needRight = std::any_of(leaves.begin(), leaves.end(), [&](const Cell &other) {
      return touchesRight(cell, other) && other.level > cell.level;
    });

    const bool needBottom = std::any_of(leaves.begin(), leaves.end(), [&](const Cell &other) {
      return touchesBottom(cell, other) && other.level > cell.level;
    });

    const bool needTop = std::any_of(leaves.begin(), leaves.end(), [&](const Cell &other) {
      return touchesTop(cell, other) && other.level > cell.level;
    });

    const std::uint64_t half = cell.size / 2;

    std::vector<GridPoint> boundary;
    boundary.reserve(8);
    boundary.push_back({cell.gx, cell.gy});
    if (needBottom) {
      boundary.push_back({cell.gx + half, cell.gy});
    }
    boundary.push_back({cell.gx + cell.size, cell.gy});
    if (needRight) {
      boundary.push_back({cell.gx + cell.size, cell.gy + half});
    }
    boundary.push_back({cell.gx + cell.size, cell.gy + cell.size});
    if (needTop) {
      boundary.push_back({cell.gx + half, cell.gy + cell.size});
    }
    boundary.push_back({cell.gx, cell.gy + cell.size});
    if (needLeft) {
      boundary.push_back({cell.gx, cell.gy + half});
    }

    const GridPoint center{cell.gx + half, cell.gy + half};
    const unsigned int centerIndex =
        vertexIndexFor(center, cache, mesh, vertexMap);

    std::vector<unsigned int> ring;
    ring.reserve(boundary.size());
    for (const GridPoint &point : boundary) {
      ring.push_back(vertexIndexFor(point, cache, mesh, vertexMap));
    }

    for (std::size_t i = 0; i < ring.size(); ++i) {
      const std::size_t next = (i + 1) % ring.size();
      mesh.indices.push_back(centerIndex);
      mesh.indices.push_back(ring[i]);
      mesh.indices.push_back(ring[next]);
    }
  }

  const auto totalEnd = std::chrono::steady_clock::now();
  const auto totalMs = std::chrono::duration_cast<std::chrono::microseconds>(
                           totalEnd - totalStart)
                           .count() /
                       1000.0;

  mesh.minZ = cache.minZ;
  mesh.maxZ = cache.maxZ;
  mesh.subdivisions = leaves.size();
  mesh.maxDepth = maxDepth;
  mesh.generationMs = totalMs;

  return mesh;
}
