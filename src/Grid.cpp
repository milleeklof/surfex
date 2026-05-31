#include "Grid.h"
#include "OpenGLUtils.h"
#include <vector>
#include <glad/glad.h>

static std::vector<float> generateGridVertices(float xmin,
                                                float xmax,
                                                float ymin,
                                                float ymax,
                                                float spacing)
{
    std::vector<float> v;

    const float color = 0.6f;

    for (float x = xmin; x <= xmax; x += spacing)
    {
        // line parallel to Y
        v.insert(v.end(), { x, ymin, 0.0f, color, color, color });
        v.insert(v.end(), { x, ymax, 0.0f, color, color, color });
    }

    for (float y = ymin; y <= ymax; y += spacing)
    {
        // line parallel to X
        v.insert(v.end(), { xmin, y, 0.0f, color, color, color });
        v.insert(v.end(), { xmax, y, 0.0f, color, color, color });
    }

    return v;
}


// Grid uses the same simple color shader as Axis.
static const char* gridVertexShaderSrc = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vColor;
uniform mat4 MVP;

void main()
{
    vColor = aColor;
    gl_Position = MVP * vec4(aPos, 1.0);
}
)";

static const char* gridFragmentShaderSrc = R"(
#version 330 core
in vec3 vColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(vColor, 0.25);
}
)";

Grid::Grid(float xmin, float xmax, float ymin, float ymax, float spacing)
{
    auto vertices = generateGridVertices(xmin, xmax, ymin, ymax, spacing);
    vertexCount = static_cast<int>(vertices.size() / 6);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    try {
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                     vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                              (void *)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                              (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);

        const unsigned int vs =
            compileShaderOrThrow(GL_VERTEX_SHADER, gridVertexShaderSrc,
                                 "Grid vertex");
        const unsigned int fs =
            compileShaderOrThrow(GL_FRAGMENT_SHADER, gridFragmentShaderSrc,
                                 "Grid fragment");
        shaderProgram = linkProgramOrThrow(vs, fs, "Grid");
        glDeleteShader(vs);
        glDeleteShader(fs);
    } catch (...) {
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
        throw;
    }
}

Grid::~Grid()
{
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);
}



void Grid::draw(const glm::mat4& MVP) const
{
    glUseProgram(shaderProgram);

    unsigned int loc = glGetUniformLocation(shaderProgram, "MVP");
    glUniformMatrix4fv(loc, 1, GL_FALSE, &MVP[0][0]);

    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, vertexCount);
    glBindVertexArray(0);
}
