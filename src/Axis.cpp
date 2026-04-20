#include "Axis.h"
#include <glad/glad.h>


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

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(axisVertices),
        axisVertices,
        GL_STATIC_DRAW
    );

    // position
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE,
        6 * sizeof(float),
        (void*)0
    );
    glEnableVertexAttribArray(0);

    // color
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE,
        6 * sizeof(float),
        (void*)(3 * sizeof(float))
    );
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // compile shaders
    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &axisVertexShaderSrc, nullptr);
    glCompileShader(vs);

    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &axisFragmentShaderSrc, nullptr);
    glCompileShader(fs);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);

    glDeleteShader(vs);
    glDeleteShader(fs);
}

Axis::~Axis()
{
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);
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
