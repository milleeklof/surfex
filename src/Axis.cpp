#include "Axis.h"
#include "OpenGLUtils.h"
#include <glad/glad.h>

namespace {

struct ShaderGuard {
    explicit ShaderGuard(unsigned int shaderId) : id(shaderId) {}
    ~ShaderGuard() {
        if (id != 0) {
            glDeleteShader(id);
        }
    }

    ShaderGuard(const ShaderGuard&) = delete;
    ShaderGuard& operator=(const ShaderGuard&) = delete;

    unsigned int id;
};

} // namespace


static const float axisVertices[] = {
    0.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,

    0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,

    0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,

    1.08f, -0.06f, 0.0f,  1.0f, 0.0f, 0.0f,
    1.18f,  0.06f, 0.0f,  1.0f, 0.0f, 0.0f,
    1.08f,  0.06f, 0.0f,  1.0f, 0.0f, 0.0f,
    1.18f, -0.06f, 0.0f,  1.0f, 0.0f, 0.0f,

   -0.06f, 1.18f, 0.0f,  0.0f, 1.0f, 0.0f,
    0.00f, 1.10f, 0.0f,  0.0f, 1.0f, 0.0f,
    0.06f, 1.18f, 0.0f,  0.0f, 1.0f, 0.0f,
    0.00f, 1.10f, 0.0f,  0.0f, 1.0f, 0.0f,
    0.00f, 1.10f, 0.0f,  0.0f, 1.0f, 0.0f,
    0.00f, 1.00f, 0.0f,  0.0f, 1.0f, 0.0f,

   -0.06f, 0.0f, 1.18f,  0.0f, 0.0f, 1.0f,
    0.06f, 0.0f, 1.18f,  0.0f, 0.0f, 1.0f,
    0.06f, 0.0f, 1.18f,  0.0f, 0.0f, 1.0f,
   -0.06f, 0.0f, 1.02f,  0.0f, 0.0f, 1.0f,
   -0.06f, 0.0f, 1.02f,  0.0f, 0.0f, 1.0f,
    0.06f, 0.0f, 1.02f,  0.0f, 0.0f, 1.0f
};

static const char* axisVertexShaderSrc = R"(
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


static const char* axisFragmentShaderSrc = R"(
#version 330 core
in vec3 vColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(vColor, 1.0);
}
)";


Axis::Axis()
{
    vertexCount = 22;

    // VAO & VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    try {
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(axisVertices), axisVertices,
                     GL_STATIC_DRAW);

        // position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                              (void *)0);
        glEnableVertexAttribArray(0);

        // color
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                              (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);

        const ShaderGuard vsGuard(
            compileShaderOrThrow(GL_VERTEX_SHADER, axisVertexShaderSrc,
                                 "Axis vertex"));
        const ShaderGuard fsGuard(
            compileShaderOrThrow(GL_FRAGMENT_SHADER, axisFragmentShaderSrc,
                                 "Axis fragment"));
        shaderProgram = linkProgramOrThrow(vsGuard.id, fsGuard.id, "Axis");
    } catch (...) {
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
        throw;
    }
}

Axis::~Axis()
{
    if (VBO != 0) {
        glDeleteBuffers(1, &VBO);
    }
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
    }
    if (shaderProgram != 0) {
        glDeleteProgram(shaderProgram);
    }
}


void Axis::draw(const glm::mat4& MVP) const
{
    glUseProgram(shaderProgram);

    unsigned int loc = glGetUniformLocation(shaderProgram, "MVP");
    glUniformMatrix4fv(loc, 1, GL_FALSE, &MVP[0][0]);

    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, 6);
    glDrawArrays(GL_LINES, 6, vertexCount - 6);
    glBindVertexArray(0);
}
