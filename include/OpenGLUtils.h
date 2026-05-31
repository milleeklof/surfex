#pragma once

#include <glad/glad.h>

unsigned int compileShaderOrThrow(GLenum type, const char *source,
                                  const char *label);
unsigned int linkProgramOrThrow(unsigned int vertexShader,
                                unsigned int fragmentShader,
                                const char *label);
