#include "Shader.h"

#include "OpenGLUtils.h"

#include <glad/glad.h>



Shader::Shader(const char* vertexSrc, const char* fragmentSrc)
{
    const unsigned int vertexShader =
        compileShaderOrThrow(GL_VERTEX_SHADER, vertexSrc, "Vertex");
    const unsigned int fragmentShader =
        compileShaderOrThrow(GL_FRAGMENT_SHADER, fragmentSrc, "Fragment");

    try {
        programID = linkProgramOrThrow(vertexShader, fragmentShader, "Program");
    } catch (...) {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        throw;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::~Shader()
{
    if (programID != 0) {
        glDeleteProgram(programID);
    }
}

void Shader::use() const
{
    glUseProgram(programID);
}

void Shader::setMat4(const char* name, const glm::mat4& mat) const
{
    unsigned int location = glGetUniformLocation(programID, name);
    glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]);
}

void Shader::setInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(programID, name.c_str()), value);
}

void Shader::setVec3(const std::string& name,
                     const glm::vec3& value) const
{
    glUniform3fv(glGetUniformLocation(programID, name.c_str()),
                 1,
                 &value[0]);
}

void Shader::setFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(programID, name.c_str()), value);
}

