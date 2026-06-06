#include "Surfex.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include <glm/gtc/type_ptr.hpp>
#include <png.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace {
bool hasLastWindowPosition = false;
int lastWindowX = 0;
int lastWindowY = 0;
int screenshotCounter = 1;

const std::unordered_map<std::string, glm::vec3> &namedColors() {
  static const std::unordered_map<std::string, glm::vec3> colors = {
      {"r", {1.0f, 0.0f, 0.0f}},
      {"g", {0.0f, 0.5f, 0.0f}},
      {"b", {0.0f, 0.0f, 1.0f}},
      {"c", {0.0f, 0.75f, 0.75f}},
      {"m", {0.75f, 0.0f, 0.75f}},
      {"y", {0.75f, 0.75f, 0.0f}},
      {"k", {0.0f, 0.0f, 0.0f}},
      {"w", {1.0f, 1.0f, 1.0f}},

      {"black", {0.0f, 0.0f, 0.0f}},
      {"dimgray", {0.4118f, 0.4118f, 0.4118f}},
      {"gray", {0.5f, 0.5f, 0.5f}},
      {"darkgray", {0.6627f, 0.6627f, 0.6627f}},
      {"silver", {0.7529f, 0.7529f, 0.7529f}},
      {"lightgray", {0.8275f, 0.8275f, 0.8275f}},
      {"gainsboro", {0.8627f, 0.8627f, 0.8627f}},
      {"whitesmoke", {0.9608f, 0.9608f, 0.9608f}},
      {"white", {1.0f, 1.0f, 1.0f}},

      {"red", {1.0f, 0.0f, 0.0f}},
      {"darkred", {0.5451f, 0.0f, 0.0f}},
      {"firebrick", {0.6980f, 0.1333f, 0.1333f}},
      {"crimson", {0.8627f, 0.0784f, 0.2353f}},
      {"salmon", {0.9804f, 0.5020f, 0.4471f}},
      {"lightsalmon", {1.0f, 0.6275f, 0.4784f}},
      {"tomato", {1.0f, 0.3882f, 0.2784f}},
      {"coral", {1.0f, 0.4980f, 0.3137f}},
      {"orangered", {1.0f, 0.2706f, 0.0f}},
      {"pink", {1.0f, 0.7529f, 0.7961f}},
      {"hotpink", {1.0f, 0.4118f, 0.7059f}},
      {"deeppink", {1.0f, 0.0784f, 0.5765f}},
      {"magenta", {1.0f, 0.0f, 1.0f}},
      {"fuchsia", {1.0f, 0.0f, 1.0f}},
      {"purple", {0.5019f, 0.0f, 0.5019f}},
      {"violet", {0.9333f, 0.5098f, 0.9333f}},
      {"orchid", {0.8549f, 0.4392f, 0.8392f}},
      {"plum", {0.8667f, 0.6275f, 0.8667f}},
      {"indigo", {0.2941f, 0.0f, 0.5098f}},

      {"blue", {0.0f, 0.0f, 1.0f}},
      {"darkblue", {0.0f, 0.0f, 0.5451f}},
      {"navy", {0.0f, 0.0f, 0.5020f}},
      {"royalblue", {0.2549f, 0.4118f, 0.8824f}},
      {"dodgerblue", {0.1176f, 0.5647f, 1.0f}},
      {"deepskyblue", {0.0f, 0.7490f, 1.0f}},
      {"skyblue", {0.5294f, 0.8078f, 0.9216f}},
      {"lightskyblue", {0.5294f, 0.8078f, 0.9804f}},
      {"cyan", {0.0f, 1.0f, 1.0f}},
      {"aqua", {0.0f, 1.0f, 1.0f}},
      {"turquoise", {0.2510f, 0.8784f, 0.8157f}},
      {"teal", {0.0f, 0.5020f, 0.5020f}},

      {"green", {0.0f, 0.5f, 0.0f}},
      {"darkgreen", {0.0f, 0.3922f, 0.0f}},
      {"lime", {0.0f, 1.0f, 0.0f}},
      {"limegreen", {0.1961f, 0.8039f, 0.1961f}},
      {"seagreen", {0.1804f, 0.5451f, 0.3412f}},
      {"forestgreen", {0.1333f, 0.5451f, 0.1333f}},
      {"olive", {0.5020f, 0.5020f, 0.0f}},
      {"olivedrab", {0.4196f, 0.5569f, 0.1373f}},
      {"yellowgreen", {0.6039f, 0.8039f, 0.1961f}},
      {"yellow", {1.0f, 1.0f, 0.0f}},
      {"gold", {1.0f, 0.8431f, 0.0f}},
      {"khaki", {0.9412f, 0.9020f, 0.5490f}},
      {"beige", {0.9608f, 0.9608f, 0.8627f}},

      {"brown", {0.6471f, 0.1647f, 0.1647f}},
      {"saddlebrown", {0.5451f, 0.2706f, 0.0745f}},
      {"chocolate", {0.8235f, 0.4118f, 0.1176f}},
      {"sienna", {0.6275f, 0.3216f, 0.1765f}},
      {"peru", {0.8039f, 0.5216f, 0.2471f}},
      {"tan", {0.8235f, 0.7059f, 0.5490f}},

      {"orange", {1.0f, 0.6471f, 0.0f}},
      {"darkorange", {1.0f, 0.5490f, 0.0f}},

      {"tab:blue", {0.1216f, 0.4667f, 0.7059f}},
      {"tab:orange", {1.0f, 0.4980f, 0.0549f}},
      {"tab:green", {0.1725f, 0.6275f, 0.1725f}},
      {"tab:red", {0.8392f, 0.1529f, 0.1569f}},
      {"tab:purple", {0.5804f, 0.4039f, 0.7412f}},
      {"tab:brown", {0.5490f, 0.3373f, 0.2941f}},
      {"tab:pink", {0.8902f, 0.4667f, 0.7608f}},
      {"tab:gray", {0.4980f, 0.4980f, 0.4980f}},
      {"tab:olive", {0.7373f, 0.7412f, 0.1333f}},
      {"tab:cyan", {0.0902f, 0.7451f, 0.8118f}},

      {"heatmap", {0.0f, 0.0f, 0.0f}}};

  return colors;
}

glm::vec3 parseColorSpec(const std::string &color) {
  const auto &colors = namedColors();
  const auto it = colors.find(color);
  if (it != colors.end()) {
    return it->second;
  }

  return glm::vec3(0.2f, 0.6f, 1.0f);
}

std::filesystem::path packageDir() {
  py::gil_scoped_acquire gil;
  static const std::filesystem::path dir = []() {
    py::module_ core = py::module_::import("surfex._core");
    return std::filesystem::path(
        py::cast<std::string>(core.attr("_package_dir")));
  }();
  return dir;
}

std::string readTextFile(const std::filesystem::path &path) {
  std::ifstream file(path);
  if (!file) {
    throw std::runtime_error("Failed to open resource file: " + path.string());
  }

  return std::string(std::istreambuf_iterator<char>(file),
                     std::istreambuf_iterator<char>());
}

std::string loadShaderText(const std::string &relativePath) {
  const std::filesystem::path path = packageDir() / "shaders" / relativePath;
  return readTextFile(path);
}
} // namespace

static void writePngFile(const std::string &path, int width, int height,
                         const std::vector<unsigned char> &pixels) {
  FILE *file = std::fopen(path.c_str(), "wb");
  if (!file) {
    throw std::runtime_error("Failed to open screenshot file.");
  }

  png_structp pngPtr =
      png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
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
  png_set_IHDR(pngPtr, infoPtr, width, height, 8, PNG_COLOR_TYPE_RGBA,
               PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);
  png_write_info(pngPtr, infoPtr);

  std::vector<png_bytep> rows(static_cast<std::size_t>(height));
  for (int y = 0; y < height; ++y) {
    rows[static_cast<std::size_t>(y)] = const_cast<png_bytep>(
        pixels.data() + static_cast<std::size_t>(height - 1 - y) *
                            static_cast<std::size_t>(width) * 4);
  }

  png_write_image(pngPtr, rows.data());
  png_write_end(pngPtr, nullptr);
  png_destroy_write_struct(&pngPtr, &infoPtr);
  std::fclose(file);
}

glm::vec3 Surfex::colorFromName(const std::string &color) {
  return parseColorSpec(color);
}

bool Surfex::isHeatmap(const std::string &color) { return color == "heatmap"; }

Surfex::Surfex(std::array<float, 2> xRange, std::array<float, 2> yRange) {
  this->xmin = xRange[0];
  this->xmax = xRange[1];
  this->ymin = yRange[0];
  this->ymax = yRange[1];
}

Surfex::~Surfex() { cleanup(); }

void Surfex::setTitle(std::string newTitle) {
  this->title = std::move(newTitle);
  if (window) {
    glfwSetWindowTitle(window, title.c_str());
  }
}

Surfex::Surface Surfex::add(Function2D func, const std::string &color,
                            float alpha) {
  return addNamed(func, "function", color, alpha);
}

Surfex::Surface Surfex::add(Function2D func, std::array<float, 2> xRange,
                            std::array<float, 2> yRange,
                            const std::string &color, float alpha) {
  return addNamed(func, xRange, yRange, "function", color, alpha);
}

void Surfex::setResolution(int n) {
  if (n < 2) {
    throw std::invalid_argument("Resolution must be between 2 and " +
                                std::to_string(kMaxSurfaceResolution) + ".");
  }
  if (n > kMaxSurfaceResolution) {
    throw std::invalid_argument("Resolution must be between 2 and " +
                                std::to_string(kMaxSurfaceResolution) + ".");
  }

  this->n = n;
}

void Surfex::setWindowSize(int width, int height) {
  if (width <= 0 || height <= 0) {
    throw std::invalid_argument("Window size must be positive.");
  }

  this->windowWidth = width;
  this->windowHeight = height;
}

Surfex::Surface Surfex::addNamed(Function2D func,
                                 const std::string &functionName,
                                 const std::string &color, float alpha) {
  if (n < 2 || n > kMaxSurfaceResolution) {
    throw std::invalid_argument("Resolution is invalid.");
  }

  return addNamed(func, std::array<float, 2>{xmin, xmax},
                   std::array<float, 2>{ymin, ymax}, functionName, color, alpha);
}

Surfex::Surface Surfex::addNamed(Function2D func, std::array<float, 2> xRange,
                                 std::array<float, 2> yRange,
                                 const std::string &functionName,
                                 const std::string &color, float alpha) {
  Surface surface;
  surface.functionName = functionName;
  const auto start = std::chrono::steady_clock::now();
  surface.mesh =
      generateSurfaceMesh(func, xRange[0], xRange[1], yRange[0], yRange[1], n);
  const auto elapsed = std::chrono::steady_clock::now() - start;
  meshGenerationMs +=
      std::chrono::duration<double, std::milli>(elapsed).count();
  surface.color = color;
  surface.alpha = alpha;
  surfaces.push_back(surface);
  return surface;
}

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

  std::vector<unsigned char> pixels(static_cast<std::size_t>(width) *
                                    static_cast<std::size_t>(height) * 4);

  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadBuffer(GL_BACK);
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

  char filename[256];
  std::snprintf(filename, sizeof(filename), "screenshots/surfex_%04d.png",
                screenshotCounter++);

  writePngFile(filename, width, height, pixels);
}

void Surfex::keyCallback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
  (void)scancode;
  (void)mods;

  if (action != GLFW_PRESS) {
    return;
  }

  Surfex *self = static_cast<Surfex *>(glfwGetWindowUserPointer(window));
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

  this->window = glfwCreateWindow(windowWidth, windowHeight, title.c_str(),
                                  nullptr, nullptr);

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

  const std::string vertexShaderSource = loadShaderText("surface.vert");
  const std::string fragmentShaderSource = loadShaderText("surface.frag");
  this->surfaceShader = std::make_unique<Shader>(vertexShaderSource.c_str(),
                                                  fragmentShaderSource.c_str());
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
  this->camera.target =
      glm::vec3((xmin + xmax) * 0.5f, (ymin + ymax) * 0.5f, 0.0f);
}

void Surfex::createGrid() {
  this->grid = std::make_unique<Grid>(xmin, xmax, ymin, ymax, 1.0f);
}

void Surfex::createAxis() { this->axis = std::make_unique<Axis>(); }

void Surfex::createBuffers() {
  for (std::size_t i = 0; i < surfaces.size(); ++i) {
    Surface &surface = surfaces[i];

    glGenVertexArrays(1, &surface.VAO);
    glGenBuffers(1, &surface.VBO);
    glGenBuffers(1, &surface.EBO);

    glBindVertexArray(surface.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, surface.VBO);
    glBufferData(GL_ARRAY_BUFFER, surface.mesh.vertices.size() * sizeof(float),
                 surface.mesh.vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, surface.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 surface.mesh.indices.size() * sizeof(unsigned int),
                 surface.mesh.indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
  }

  glBindVertexArray(0);
}

void Surfex::printSummary(double elapsedMs) const {
  if (surfaces.empty()) {
    return;
  }

  std::vector<std::string> names;
  names.reserve(surfaces.size());
  for (const Surface &surface : surfaces) {
    if (std::find(names.begin(), names.end(), surface.functionName) ==
        names.end()) {
      names.push_back(surface.functionName);
    }
  }

  std::cout << "=== \"" << title << "\":===\n";
  std::cout << "\tFunction(s): ";
  for (std::size_t i = 0; i < names.size(); ++i) {
    if (i != 0) {
      std::cout << ", ";
    }
    std::cout << names[i];
  }
  std::cout << "\n\tMesh generation time: " << elapsedMs << " ms\n";
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
    Surface &surface = surfaces[i];

    surfaceShader->setFloat("minZ", surface.mesh.minZ);
    surfaceShader->setFloat("maxZ", surface.mesh.maxZ);
    surfaceShader->setInt("renderMode", isHeatmap(surface.color) ? 1 : 0);
    surfaceShader->setVec3("baseColor", colorFromName(surface.color));
    surfaceShader->setFloat("alpha", surface.alpha);

    glBindVertexArray(surface.VAO);
    glDrawElements(GL_TRIANGLES,
                   static_cast<GLsizei>(surface.mesh.indices.size()),
                   GL_UNSIGNED_INT, 0);
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
    Surface &surface = surfaces[i];

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

  axis.reset();
  grid.reset();
  surfaceShader.reset();

  if (window) {
    glfwGetWindowPos(window, &lastWindowX, &lastWindowY);
    hasLastWindowPosition = true;
    glfwDestroyWindow(window);
    window = nullptr;
    glfwTerminate();
  }

  orientationAnimating = false;
  saveScreenshotRequested = false;
  targetYaw = 0.0f;
  targetPitch = 0.0f;
  summaryPrinted = false;
  meshGenerationMs = 0.0;
}

void Surfex::run() {
  initWindow();
  try {
    summaryPrinted = false;
    meshGenerationMs = 0.0;
    saveScreenshotRequested = false;
    orientationAnimating = false;
    initGL();
    createCamera();
    targetYaw = camera.yaw;
    targetPitch = camera.pitch;
    createAxis();
    createGrid();
    createBuffers();

    if (!summaryPrinted && !surfaces.empty()) {
      printSummary(meshGenerationMs);
      summaryPrinted = true;
    }

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
