#pragma once

#include <chrono>
#include <array>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Axis.h"
#include "Camera.h"
#include "Grid.h"
#include "Mesh.h"
#include "Shader.h"

class Surfex {
public:
  using Function2D = std::function<float(float, float)>;

  struct Surface {
    Mesh mesh;
    std::string functionName;
    std::string color;
    float alpha = 1.0f;
    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;
  };

  Surfex(std::array<float, 2> xRange, std::array<float, 2> yRange);

  ~Surfex();

  void setTitle(std::string title);

  Surface add(Function2D func, const std::string &color = "blue",
              float alpha = 1.0f);

  Surface add(Function2D func, std::array<float, 2> xRange,
              std::array<float, 2> yRange, const std::string &color = "blue",
              float alpha = 1.0f);

  Surface addNamed(Function2D func, const std::string &functionName,
                   const std::string &color = "blue", float alpha = 1.0f);

  Surface addNamed(Function2D func, std::array<float, 2> xRange,
                   std::array<float, 2> yRange,
                   const std::string &functionName,
                   const std::string &color = "blue", float alpha = 1.0f);

  void run();

  void setResolution(int n);
  void setWindowSize(int width, int height);

private:
  static void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                          int mods);

  void initWindow();
  void initGL();
  void createCamera();
  void createGrid();
  void createAxis();
  void createBuffers();
  void printSummary(double elapsedMs) const;
  void processInput(float deltaTime);
  void renderFrame();
  void cleanup();
  void setTargetOrientation(float yaw, float pitch);
  void updateOrientation(float deltaTime);
  void saveScreenshot();

  static glm::vec3 colorFromName(const std::string &color);
  static bool isHeatmap(const std::string &color);

  float xmin = -15.0f;
  float xmax = 15.0f;
  float ymin = -15.0f;
  float ymax = 15.0f;

  int n = 500;

  int windowWidth = 800;
  int windowHeight = 600;
  std::string title = "Surfex";

  bool showAxis = true;
  bool showGrid = true;
  bool orientationAnimating = false;
  bool saveScreenshotRequested = false;
  bool summaryPrinted = false;
  float targetYaw = 0.0f;
  float targetPitch = 0.0f;

  double meshGenerationMs = 0.0;

  GLFWwindow *window = nullptr;

  std::vector<Surface> surfaces;
  Camera camera{};
  std::unique_ptr<Axis> axis;
  std::unique_ptr<Grid> grid;
  std::unique_ptr<Shader> surfaceShader;
};
