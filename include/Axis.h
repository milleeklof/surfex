#pragma once
#include <glm/glm.hpp>

struct Axis
{
    Axis();
    ~Axis();

    void draw(const glm::mat4& MVP) const;

private:
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int shaderProgram = 0;
    int vertexCount = 0;
};
