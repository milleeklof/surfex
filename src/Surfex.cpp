#include "Surfex.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <vector>

#include <glm/gtc/type_ptr.hpp>
#include <png.h>

namespace {
bool hasLastWindowPosition = false;
int lastWindowX = 0;
int lastWindowY = 0;
int screenshotCounter = 1;

const char *vertexShaderSource = R"(
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
)";

const char *fragmentShaderSource = R"(
#version 330 core

uniform float minZ;
uniform float maxZ;

in vec3 vNormal;
in float vHeight;

out vec4 FragColor;

uniform vec3 lightDir;
uniform vec3 baseColor;
uniform int renderMode;
uniform float alpha;

vec3 heatmap(float t)
{
    t = clamp(t, 0.0, 1.0);

    vec3 c1 = vec3(0.0, 0.0, 0.5);
    vec3 c2 = vec3(0.0, 0.0, 1.0);
    vec3 c3 = vec3(0.0, 1.0, 0.0);
    vec3 c4 = vec3(1.0, 1.0, 0.0);
    vec3 c5 = vec3(1.0, 0.0, 0.0);

    if (t < 0.25)
        return mix(c1, c2, t / 0.25);
    else if (t < 0.5)
        return mix(c2, c3, (t - 0.25) / 0.25);
    else if (t < 0.75)
        return mix(c3, c4, (t - 0.5) / 0.25);
    else
        return mix(c4, c5, (t - 0.75) / 0.25);
}

void main()
{
    vec3 n = normalize(vNormal);
    vec3 l = normalize(lightDir);

    float diff = max(dot(n, l), 0.0);

    vec3 color;

    if (renderMode == 0)
    {
        color = baseColor;
    }
    else
    {
        float range = max(maxZ - minZ, 0.0001);
        float normalizedHeight = (vHeight - minZ) / range;
        normalizedHeight = clamp(normalizedHeight, 0.0, 1.0);
        color = heatmap(normalizedHeight);
    }

    vec3 ambient = 0.2 * color;
    vec3 diffuse = diff * color;

    FragColor = vec4(ambient + diffuse, alpha);
}
)";
} // namespace

static void writePngFile(const std::string& path,
                         int width,
                         int height,
                         const std::vector<unsigned char>& pixels) {
  FILE* file = std::fopen(path.c_str(), "wb");
  if (!file) {
    throw std::runtime_error("Failed to open screenshot file.");
  }

  png_structp pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if (!pngPtr) {
    std::fclose(file);
    throw std::runtime_error("Failed to create PNG writer.");
  }

  png_infop infoPtr = png_create_info_struct(pngPtr);
  if (!infoPtr) {
    png_destroy_write_struct(&pngPtr, nullptr);
    std::fclose(file);
    throw std::runtime_error("Failed to create PNG info struct.");
  }

  if (setjmp(png_jmpbuf(pngPtr))) {
    png_destroy_write_struct(&pngPtr, &infoPtr);
    std::fclose(file);
    throw std::runtime_error("Failed to write PNG file.");
  }

  png_init_io(pngPtr, file);
  png_set_IHDR(pngPtr,
               infoPtr,
               width,
               height,
               8,
               PNG_COLOR_TYPE_RGBA,
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);
  png_write_info(pngPtr, infoPtr);

  std::vector<png_bytep> rows(static_cast<std::size_t>(height));
  for (int y = 0; y < height; ++y) {
    rows[static_cast<std::size_t>(y)] = const_cast<png_bytep>(
        pixels.data() + static_cast<std::size_t>(height - 1 - y) * static_cast<std::size_t>(width) * 4);
  }

  png_write_image(pngPtr, rows.data());
  png_write_end(pngPtr, nullptr);
  png_destroy_write_struct(&pngPtr, &infoPtr);
  std::fclose(file);
}

glm::vec3 Surfex::colorFromName(const std::string& color) {
  if (color == "red") {
    return glm::vec3(1.0f, 0.2f, 0.2f);
  }

  if (color == "green") {
    return glm::vec3(0.2f, 1.0f, 0.3f);
  }

  if (color == "yellow") {
    return glm::vec3(1.0f, 0.9f, 0.2f);
  }

  return glm::vec3(0.2f, 0.6f, 1.0f);
}

bool Surfex::isHeatmap(const std::string& color) {
  return color == "heatmap";
}

Surfex::Surfex(std::array<float, 2> xRange, std::array<float, 2> yRange) {
  this->xmin = xRange[0];
  this->xmax = xRange[1];
  this->ymin = yRange[0];
  this->ymax = yRange[1];
}

Surfex::~Surfex() { cleanup(); }

Surfex::Surface Surfex::add(Function2D func, const std::string& color, float alpha) {
  return add(func, std::array<float, 2>{xmin, xmax}, std::array<float, 2>{ymin, ymax}, color, alpha);
}

Surfex::Surface Surfex::add(Function2D func,
                            std::array<float, 2> xRange,
                            std::array<float, 2> yRange,
                            const std::string& color,
                            float alpha) {
  Surface surface;
  surface.mesh = generateSurfaceMesh(func, xRange[0], xRange[1], yRange[0], yRange[1], nx, ny);
  surface.color = color;
  surface.alpha = alpha;
  surfaces.push_back(surface);
  return surface;
}

void Surfex::setResolution(int nx, int ny) {
  if (nx < 2 || ny < 2) {
    throw std::invalid_argument("Resolution must be at least 2x2.");
  }

  this->nx = nx;
  this->ny = ny;
}

void Surfex::setWindowSize(int width, int height) {
  if (width <= 0 || height <= 0) {
    throw std::invalid_argument("Window size must be positive.");
  }

  this->windowWidth = width;
  this->windowHeight = height;
}

void Surfex::setTitle(const std::string& title) { this->title = title; }

void Surfex::setTargetOrientation(float yaw, float pitch) {
  this->targetYaw = yaw;
  this->targetPitch = pitch;
  this->orientationAnimating = true;
}

void Surfex::updateOrientation(float deltaTime) {
  if (!orientationAnimating) {
    return;
  }

  const float animationSpeed = glm::radians(180.0f) * deltaTime;

  if (camera.yaw < targetYaw) {
    camera.yaw = std::min(camera.yaw + animationSpeed, targetYaw);
  } else if (camera.yaw > targetYaw) {
    camera.yaw = std::max(camera.yaw - animationSpeed, targetYaw);
  }

  if (camera.pitch < targetPitch) {
    camera.pitch = std::min(camera.pitch + animationSpeed, targetPitch);
  } else if (camera.pitch > targetPitch) {
    camera.pitch = std::max(camera.pitch - animationSpeed, targetPitch);
  }

  if (camera.yaw == targetYaw && camera.pitch == targetPitch) {
    orientationAnimating = false;
  }
}

void Surfex::saveScreenshot() {
  std::filesystem::create_directories("screenshots");

  int width = 0;
  int height = 0;
  glfwGetFramebufferSize(window, &width, &height);
  if (width <= 0 || height <= 0) {
    throw std::runtime_error("Invalid framebuffer size for screenshot.");
  }

  std::vector<unsigned char> pixels(static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4);

  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadBuffer(GL_BACK);
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

  char filename[256];
  std::snprintf(filename, sizeof(filename), "screenshots/surfex_%04d.png", screenshotCounter++);

  writePngFile(filename, width, height, pixels);
}

void Surfex::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  (void)scancode;
  (void)mods;

  if (action != GLFW_PRESS) {
    return;
  }

  Surfex* self = static_cast<Surfex*>(glfwGetWindowUserPointer(window));
  if (!self) {
    return;
  }

  if (key == GLFW_KEY_P) {
    self->saveScreenshotRequested = true;
  }
}

void Surfex::initWindow() {
  if (!glfwInit()) {
    throw std::runtime_error("Failed to initialize GLFW.");
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  this->window = glfwCreateWindow(windowWidth, windowHeight, title.c_str(), nullptr, nullptr);

  if (!window) {
    glfwTerminate();
    throw std::runtime_error("Failed to create GLFW window.");
  }

  if (hasLastWindowPosition) {
    glfwSetWindowPos(window, lastWindowX, lastWindowY);
  }

  glfwMakeContextCurrent(window);
  glfwSetWindowUserPointer(window, this);
  glfwSetKeyCallback(window, keyCallback);
}

void Surfex::initGL() {
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    throw std::runtime_error("Failed to initialize GLAD.");
  }

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  this->surfaceShader = new Shader(vertexShaderSource, fragmentShaderSource);
}

void Surfex::createCamera() {
  this->camera.pitch = 3.14f / 8.0f;
  this->camera.yaw = 3.14f / 4.0f;

  const float xSpan = xmax - xmin;
  const float ySpan = ymax - ymin;
  float maxSpan = std::max(xSpan, ySpan);
  if (maxSpan <= 0.0f) {
    maxSpan = 1.0f;
  }

  this->camera.radius = 1.5f * maxSpan;
  this->camera.target = glm::vec3((xmin + xmax) * 0.5f, (ymin + ymax) * 0.5f, 0.0f);
}

void Surfex::createGrid() {
  this->grid = new Grid(xmin, xmax, ymin, ymax, 1.0f);
}

void Surfex::createAxis() { this->axis = new Axis(); }

void Surfex::createBuffers() {
  for (std::size_t i = 0; i < surfaces.size(); ++i) {
    Surface& surface = surfaces[i];

    glGenVertexArrays(1, &surface.VAO);
    glGenBuffers(1, &surface.VBO);
    glGenBuffers(1, &surface.EBO);

    glBindVertexArray(surface.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, surface.VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 surface.mesh.vertices.size() * sizeof(float),
                 surface.mesh.vertices.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, surface.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 surface.mesh.indices.size() * sizeof(unsigned int),
                 surface.mesh.indices.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
  }

  glBindVertexArray(0);
}

void Surfex::processInput(float deltaTime) {
  const float rotateSpeed = glm::radians(90.0f) * deltaTime;
  const float xSpan = xmax - xmin;
  const float ySpan = ymax - ymin;
  const float zoomSpeed = 0.5f * std::max(xSpan, ySpan) * deltaTime;

  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS ||
      glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
    camera.yaw -= rotateSpeed;
  }

  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS ||
      glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
    camera.yaw += rotateSpeed;
  }

  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS ||
      glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
    camera.pitch += rotateSpeed;
  }

  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS ||
      glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
    camera.pitch -= rotateSpeed;
  }

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && camera.radius > 0.0f)
    camera.radius -= zoomSpeed;

  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.radius += zoomSpeed;

  if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
    setTargetOrientation(0.0f, 0.0f);
  }

  if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
    setTargetOrientation(3.1415f / 2.0f, 0.0f);
  }

  if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
    setTargetOrientation(0.0f, 3.1415f / 2.0f);
  }

  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
    return;
  }

  camera.clampPitch();
  updateOrientation(deltaTime);
  camera.clampPitch();
}

void Surfex::renderFrame() {
  glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  int width = 0;
  int height = 0;
  glfwGetFramebufferSize(window, &width, &height);
  if (height == 0) {
    height = 1;
  }
  glViewport(0, 0, width, height);

  const float aspect = static_cast<float>(width) / static_cast<float>(height);

  const glm::mat4 model = glm::mat4(1.0f);
  const glm::mat4 view = camera.viewMatrix();
  const glm::mat4 projection = camera.projectionMatrix(aspect);
  const glm::mat4 MVP = projection * view * model;

  surfaceShader->use();
  surfaceShader->setVec3("lightDir",
                         glm::normalize(glm::vec3(1.0f, 1.0f, 2.0f)));
  surfaceShader->setMat4("MVP", MVP);
  surfaceShader->setMat4("model", model);

  for (std::size_t i = 0; i < surfaces.size(); ++i) {
    Surface& surface = surfaces[i];

    surfaceShader->setFloat("minZ", surface.mesh.minZ);
    surfaceShader->setFloat("maxZ", surface.mesh.maxZ);
    surfaceShader->setInt("renderMode", isHeatmap(surface.color) ? 1 : 0);
    surfaceShader->setVec3("baseColor", colorFromName(surface.color));
    surfaceShader->setFloat("alpha", surface.alpha);

    glBindVertexArray(surface.VAO);
    glDrawElements(GL_TRIANGLES,
                   static_cast<GLsizei>(surface.mesh.indices.size()),
                   GL_UNSIGNED_INT,
                   0);
  }

  if (showAxis && axis) {
    axis->draw(MVP);
  }

  if (showGrid && grid) {
    grid->draw(MVP);
  }

  if (saveScreenshotRequested) {
    saveScreenshotRequested = false;
    saveScreenshot();
  }

  glBindVertexArray(0);
  glfwSwapBuffers(window);
  glfwPollEvents();
}

void Surfex::cleanup() {
  for (std::size_t i = 0; i < surfaces.size(); ++i) {
    Surface& surface = surfaces[i];

    if (surface.EBO != 0) {
      glDeleteBuffers(1, &surface.EBO);
      surface.EBO = 0;
    }

    if (surface.VBO != 0) {
      glDeleteBuffers(1, &surface.VBO);
      surface.VBO = 0;
    }

    if (surface.VAO != 0) {
      glDeleteVertexArrays(1, &surface.VAO);
      surface.VAO = 0;
    }
  }

  delete axis;
  axis = nullptr;
  delete grid;
  grid = nullptr;
  delete surfaceShader;
  surfaceShader = nullptr;

  if (window) {
    glfwGetWindowPos(window, &lastWindowX, &lastWindowY);
    hasLastWindowPosition = true;
    glfwDestroyWindow(window);
    window = nullptr;
    glfwTerminate();
  }
}

void Surfex::run() {
  initWindow();
  try {
    initGL();
    createCamera();
    createAxis();
    createGrid();
    createBuffers();

    float lastTime = static_cast<float>(glfwGetTime());

    while (!glfwWindowShouldClose(window)) {
      const float currentTime = static_cast<float>(glfwGetTime());
      const float deltaTime = currentTime - lastTime;
      lastTime = currentTime;

      processInput(deltaTime);
      renderFrame();
    }

    cleanup();
  } catch (...) {
    cleanup();
    throw;
  }
}
