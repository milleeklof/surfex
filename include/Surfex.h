#pragma once

#include <array>
#include <functional>
#include <memory>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Axis.h"
#include "Camera.h"
#include "Grid.h"
#include "Mesh.h"
#include "Shader.h"

class Surfex
{
public:
    using Function2D = std::function<float(float, float)>;

    Surfex(Function2D func,
           std::array<float, 2> xRange,
           std::array<float, 2> yRange);

    ~Surfex();

    void run();

    void setResolution(int nx, int ny);
    void setWindowSize(int width, int height);
    void setTitle(const std::string& title);

private:
    enum class RenderMode
    {
        Solid,
        Heatmap
    };

    enum class ColorMode
    {
        Blue,
        Red,
        Green,
        Yellow
    };

    static void keyCallback(GLFWwindow* window,
                            int key,
                            int scancode,
                            int action,
                            int mods);

    void initWindow();
    void initGL();
    void createMesh();
    void createCamera();
    void createGrid();
    void createAxis();
    void createBuffers();
    void processInput(float deltaTime);
    void renderFrame();
    void cleanup();

    Function2D function_;
    std::array<float, 2> xRange_;
    std::array<float, 2> yRange_;

    int nx_ = 300;
    int ny_ = 300;

    int windowWidth_ = 800;
    int windowHeight_ = 600;
    std::string title_ = "Surfex";

    float halfLength_ = 15.0f;

    bool showAxis_ = true;
    bool showGrid_ = true;
    RenderMode renderMode_ = RenderMode::Solid;
    ColorMode colorMode_ = ColorMode::Blue;

    GLFWwindow* window_ = nullptr;

    Mesh mesh_;
    Camera camera_{};
    std::unique_ptr<Axis> axis_;
    std::unique_ptr<Grid> grid_;
    std::unique_ptr<Shader> surfaceShader_;

    GLuint VAO_ = 0;
    GLuint VBO_ = 0;
    GLuint EBO_ = 0;
};
