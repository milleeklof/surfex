#include "Surfex.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <utility>

#include <glm/gtc/type_ptr.hpp>

namespace
{
const char* vertexShaderSource = R"(
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

const char* fragmentShaderSource = R"(
#version 330 core

uniform float minZ;
uniform float maxZ;

in vec3 vNormal;
in float vHeight;

out vec4 FragColor;

uniform vec3 lightDir;
uniform vec3 baseColor;
uniform int renderMode;

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

    FragColor = vec4(ambient + diffuse, 1.0);
}
)";
}

Surfex::Surfex(Function2D func,
               std::array<float, 2> xRange,
               std::array<float, 2> yRange)
    : function_(std::move(func)),
      xRange_(xRange),
      yRange_(yRange)
{
}

Surfex::~Surfex()
{
    cleanup();
}

void Surfex::setResolution(int nx, int ny)
{
    if (nx < 2 || ny < 2)
    {
        throw std::invalid_argument("Resolution must be at least 2x2.");
    }

    nx_ = nx;
    ny_ = ny;
}

void Surfex::setWindowSize(int width, int height)
{
    if (width <= 0 || height <= 0)
    {
        throw std::invalid_argument("Window size must be positive.");
    }

    windowWidth_ = width;
    windowHeight_ = height;
}

void Surfex::setTitle(const std::string& title)
{
    title_ = title;
}

void Surfex::keyCallback(GLFWwindow* window,
                         int key,
                         int scancode,
                         int action,
                         int mods)
{
    (void)scancode;
    (void)mods;

    if (action != GLFW_PRESS)
    {
        return;
    }

    Surfex* self = static_cast<Surfex*>(glfwGetWindowUserPointer(window));
    if (!self)
    {
        return;
    }

    if (key == GLFW_KEY_H)
    {
        self->renderMode_ = self->renderMode_ == RenderMode::Solid
            ? RenderMode::Heatmap
            : RenderMode::Solid;
    }
}

void Surfex::initWindow()
{
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialize GLFW.");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window_ = glfwCreateWindow(windowWidth_,
                               windowHeight_,
                               title_.c_str(),
                               nullptr,
                               nullptr);

    if (!window_)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window.");
    }

    glfwMakeContextCurrent(window_);
    glfwSetWindowUserPointer(window_, this);
    glfwSetKeyCallback(window_, keyCallback);
}

void Surfex::initGL()
{
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        throw std::runtime_error("Failed to initialize GLAD.");
    }

    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    surfaceShader_ = std::make_unique<Shader>(vertexShaderSource,
                                              fragmentShaderSource);
}

void Surfex::createMesh()
{
    mesh_ = generateSurfaceMesh(function_,
                                xRange_[0],
                                xRange_[1],
                                yRange_[0],
                                yRange_[1],
                                nx_,
                                ny_);

    float maxAbsX = std::max(std::abs(xRange_[0]), std::abs(xRange_[1]));
    float maxAbsY = std::max(std::abs(yRange_[0]), std::abs(yRange_[1]));
    halfLength_ = std::max(maxAbsX, maxAbsY);
    if (halfLength_ <= 0.0f)
    {
        halfLength_ = 1.0f;
    }
}

void Surfex::createCamera()
{
    camera_.pitch = 3.14f / 8.0f;
    camera_.yaw = 3.14f / 4.0f;
    camera_.radius = 3.0f * halfLength_;
    camera_.target = glm::vec3(0.0f, 0.0f, 0.0f);
}

void Surfex::createGrid()
{
    grid_ = std::make_unique<Grid>(halfLength_ * 1.5f, 1.0f);
}

void Surfex::createAxis()
{
    axis_ = std::make_unique<Axis>();
}

void Surfex::createBuffers()
{
    glGenVertexArrays(1, &VAO_);
    glGenBuffers(1, &VBO_);
    glGenBuffers(1, &EBO_);

    glBindVertexArray(VAO_);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_);
    glBufferData(GL_ARRAY_BUFFER,
                 mesh_.vertices.size() * sizeof(float),
                 mesh_.vertices.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 mesh_.indices.size() * sizeof(unsigned int),
                 mesh_.indices.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          6 * sizeof(float),
                          (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          6 * sizeof(float),
                          (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Surfex::processInput(float deltaTime)
{
    const float rotateSpeed = glm::radians(90.0f) * deltaTime;
    const float zoomSpeed = halfLength_ * deltaTime;

    if (glfwGetKey(window_, GLFW_KEY_LEFT) == GLFW_PRESS)
        camera_.yaw -= rotateSpeed;

    if (glfwGetKey(window_, GLFW_KEY_RIGHT) == GLFW_PRESS)
        camera_.yaw += rotateSpeed;

    if (glfwGetKey(window_, GLFW_KEY_UP) == GLFW_PRESS)
        camera_.pitch += rotateSpeed;

    if (glfwGetKey(window_, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera_.pitch -= rotateSpeed;

    if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS && camera_.radius > 0.0f)
        camera_.radius -= zoomSpeed;

    if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS)
        camera_.radius += zoomSpeed;

    if (glfwGetKey(window_, GLFW_KEY_X) == GLFW_PRESS)
    {
        camera_.yaw = 0.0f;
        camera_.pitch = 0.0f;
    }

    if (glfwGetKey(window_, GLFW_KEY_Y) == GLFW_PRESS)
    {
        camera_.yaw = 3.1415f / 2.0f;
        camera_.pitch = 0.0f;
    }

    if (glfwGetKey(window_, GLFW_KEY_Z) == GLFW_PRESS)
    {
        camera_.yaw = 0.0f;
        camera_.pitch = 3.1415f / 2.0f;
    }

    camera_.clampPitch();
}

void Surfex::renderFrame()
{
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(window_, &width, &height);
    if (height == 0)
    {
        height = 1;
    }
    glViewport(0, 0, width, height);

    const float aspect = static_cast<float>(width) / static_cast<float>(height);

    const glm::mat4 model = glm::mat4(1.0f);
    const glm::mat4 view = camera_.viewMatrix();
    const glm::mat4 projection = camera_.projectionMatrix(aspect);
    const glm::mat4 MVP = projection * view * model;

    surfaceShader_->use();
    surfaceShader_->setFloat("minZ", mesh_.minZ);
    surfaceShader_->setFloat("maxZ", mesh_.maxZ);
    surfaceShader_->setMat4("model", model);
    surfaceShader_->setMat4("MVP", MVP);
    surfaceShader_->setVec3("lightDir",
                            glm::normalize(glm::vec3(1.0f, 1.0f, 2.0f)));
    surfaceShader_->setInt("renderMode",
                           renderMode_ == RenderMode::Solid ? 0 : 1);

    glm::vec3 baseColor;
    switch (colorMode_)
    {
        case ColorMode::Blue:   baseColor = {0.2f, 0.6f, 1.0f}; break;
        case ColorMode::Red:    baseColor = {1.0f, 0.2f, 0.2f}; break;
        case ColorMode::Green:  baseColor = {0.2f, 1.0f, 0.3f}; break;
        case ColorMode::Yellow: baseColor = {1.0f, 0.9f, 0.2f}; break;
    }
    surfaceShader_->setVec3("baseColor", baseColor);

    glBindVertexArray(VAO_);
    glDrawElements(GL_TRIANGLES,
                   static_cast<GLsizei>(mesh_.indices.size()),
                   GL_UNSIGNED_INT,
                   0);

    if (showAxis_ && axis_)
    {
        axis_->draw(MVP);
    }

    if (showGrid_ && grid_)
    {
        grid_->draw(MVP);
    }

    glBindVertexArray(0);
    glfwSwapBuffers(window_);
    glfwPollEvents();
}

void Surfex::cleanup()
{
    axis_.reset();
    grid_.reset();
    surfaceShader_.reset();

    if (EBO_ != 0)
    {
        glDeleteBuffers(1, &EBO_);
        EBO_ = 0;
    }

    if (VBO_ != 0)
    {
        glDeleteBuffers(1, &VBO_);
        VBO_ = 0;
    }

    if (VAO_ != 0)
    {
        glDeleteVertexArrays(1, &VAO_);
        VAO_ = 0;
    }

    if (window_)
    {
        glfwDestroyWindow(window_);
        window_ = nullptr;
        glfwTerminate();
    }
}

void Surfex::run()
{
    initWindow();
    try
    {
        initGL();
        createMesh();
        createCamera();
        createAxis();
        createGrid();
        createBuffers();

        float lastTime = static_cast<float>(glfwGetTime());

        while (!glfwWindowShouldClose(window_))
        {
            const float currentTime = static_cast<float>(glfwGetTime());
            const float deltaTime = currentTime - lastTime;
            lastTime = currentTime;

            processInput(deltaTime);
            renderFrame();
        }
    }
    catch (...)
    {
        cleanup();
        throw;
    }
}
