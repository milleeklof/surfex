#pragma once
#include <glm/glm.hpp>

struct Grid
{
    Grid(float halfSize = 10.0f, float spacing = 1.0f);
    ~Grid();

    void draw(const glm::mat4& MVP) const;

private:
    unsigned int VAO;
    unsigned int VBO;
    unsigned int shaderProgram;
    int vertexCount;
};


