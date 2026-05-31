#include "OpenGLUtils.h"

#include <iostream>
#include <stdexcept>
#include <string>

namespace {

std::string shaderLog(unsigned int shader) {
  int length = 0;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
  std::string log(static_cast<std::size_t>(length), '\0');
  if (length > 0) {
    glGetShaderInfoLog(shader, length, nullptr, log.data());
  }
  return log;
}

std::string programLog(unsigned int program) {
  int length = 0;
  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
  std::string log(static_cast<std::size_t>(length), '\0');
  if (length > 0) {
    glGetProgramInfoLog(program, length, nullptr, log.data());
  }
  return log;
}

} // namespace

unsigned int compileShaderOrThrow(GLenum type, const char *source,
                                  const char *label) {
  const unsigned int shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);

  int success = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    const std::string log = shaderLog(shader);
    glDeleteShader(shader);
    throw std::runtime_error(std::string(label) + " shader compilation failed:\n" + log);
  }

  return shader;
}

unsigned int linkProgramOrThrow(unsigned int vertexShader,
                                unsigned int fragmentShader,
                                const char *label) {
  const unsigned int program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);

  int success = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    const std::string log = programLog(program);
    glDeleteProgram(program);
    throw std::runtime_error(std::string(label) + " shader linking failed:\n" + log);
  }

  return program;
}
