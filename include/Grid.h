#pragma once
#include <glm/glm.hpp>

struct Grid
{
    Grid(float xmin = -10.0f,
         float xmax = 10.0f,
         float ymin = -10.0f,
         float ymax = 10.0f,
         float spacing = 1.0f);
    ~Grid();

    void draw(const glm::mat4& MVP) const;

private:
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int shaderProgram = 0;
    int vertexCount = 0;
};
