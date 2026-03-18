#include "Shader.h"

#include <glad/glad.h>
#include <iostream>



static unsigned int compileShader(unsigned int type, const char* source)
{
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation error:\n" << infoLog << std::endl;
    }

    return shader;
}

Shader::Shader(const char* vertexSrc, const char* fragmentSrc)
{
    unsigned int vertexShader   = compileShader(GL_VERTEX_SHADER, vertexSrc);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSrc);

    programID = glCreateProgram();
    glAttachShader(programID, vertexShader);
    glAttachShader(programID, fragmentShader);
    glLinkProgram(programID);

    int success;
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(programID, 512, nullptr, infoLog);
        std::cerr << "Shader linking error:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::~Shader()
{
    glDeleteProgram(programID);
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



