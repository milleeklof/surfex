#include"Mesh.h"
#include<cmath>
#include<functional>
#include <glm/gtc/type_ptr.hpp>
#include <limits> // for max/min

Mesh generateSurfaceMesh(
    std::function<float(float, float)> f,
    float xmin, float xmax,
    float ymin, float ymax,
    int Nx, int Ny
)
{
    Mesh mesh;

    mesh.vertices.reserve(Nx * Ny * 6);
    mesh.indices.reserve((Nx - 1) * (Ny - 1) * 6);

    // Calcualte maximum and minimum (for heatmap)
    mesh.minZ = std::numeric_limits<float>::max(); // Starts out extremely high
    mesh.maxZ = -std::numeric_limits<float>::max(); // Starts out extremely low

    // Discretize the space into points
    for(int i = 0; i < Nx; i++)
    {
        for(int j = 0; j < Ny; j++)
        {
            float x = xmin + i * (xmax - xmin) / (Nx - 1);
            float y = ymin + j * (ymax - ymin) / (Ny - 1);
            float z = f(x, y);

            // Save Z value if it is smaller than max och greater than min
            mesh.minZ = std::min(mesh.minZ, z);
            mesh.maxZ = std::max(mesh.maxZ, z);

            // Derivatives, for Lambert shading
            float h = 0.001f;
            float fx = (f(x + h, y) - f(x - h, y)) / (2*h);
            float fy = (f(x, y + h) - f(x, y - h)) / (2*h);

            mesh.vertices.push_back(x);
            mesh.vertices.push_back(y);
            mesh.vertices.push_back(z);

            glm::vec3 normal(-fx, -fy, 1.0f); // Normal vector to surface
            normal = glm::normalize(normal); // Normalize it

            mesh.vertices.push_back(normal.x);
            mesh.vertices.push_back(normal.y);
            mesh.vertices.push_back(normal.z);
        }
    }

    // Create triangles (needed for OpenGL)
    for (int j = 0; j < Ny - 1; ++j)
    {
        for (int i = 0; i < Nx - 1; ++i)
        {
            int row1 = j * Nx;
            int row2 = (j + 1) * Nx;

            // Each square consists of two triangles:

            // Triangle 1
            mesh.indices.push_back(row1 + i);
            mesh.indices.push_back(row2 + i);
            mesh.indices.push_back(row1 + i + 1);

            // Triangle 2
            mesh.indices.push_back(row1 + i + 1);
            mesh.indices.push_back(row2 + i);
            mesh.indices.push_back(row2 + i + 1);
        }
    }
    return mesh;
}

