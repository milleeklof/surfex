#pragma once
#include <glm/glm.hpp>

struct Axis
{
    Axis();
    ~Axis();

    void draw(const glm::mat4& MVP) const;

private:
    unsigned int VAO;
    unsigned int VBO;
    unsigned int shaderProgram;
};
