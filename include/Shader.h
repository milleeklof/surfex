#pragma once
#include <glm/glm.hpp>
#include <string>


struct Shader
{
    Shader(const char* vertexSrc, const char* fragmentSrc);
    ~Shader();

    void use() const;
    void setMat4(const char* name, const glm::mat4& mat) const;
    void setInt(const std::string& name, int value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setFloat(const std::string& name, float value) const;

private:
    unsigned int programID;
};
