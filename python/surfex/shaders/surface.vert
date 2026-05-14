#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 MVP;
uniform mat4 model;

out vec3 vNormal;
out float vHeight;

void main()
{
    vNormal = mat3(transpose(inverse(model))) * aNormal;
    vHeight = aPos.z;

    gl_Position = MVP * vec4(aPos, 1.0);
}
