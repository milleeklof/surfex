#include <glad/glad.h> // GLAD has to be included first
#include <GLFW/glfw3.h>
#include <iostream>
#include "Camera.h"
#include "Mesh.h"
#include "Shader.h"
#include "Axis.h"
#include "Grid.h"
#include <glm/gtc/type_ptr.hpp>


#include <cmath>

// =================================
//        GLOBAL VARIABLES
// =================================

bool showAxis = true;
bool showGrid = true;

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

RenderMode renderMode = RenderMode::Solid;
ColorMode colorMode = ColorMode::Blue;


// =================================
//        CHECK H PRESSED
// =================================
void keyCallback(GLFWwindow* window,
                 int key,
                 int scancode,
                 int action,
                 int mods)
{
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_H)
        {
            RenderMode* mode =
                static_cast<RenderMode*>(
                    glfwGetWindowUserPointer(window));

            if (*mode == RenderMode::Solid)
                *mode = RenderMode::Heatmap;
            else
                *mode = RenderMode::Solid;
        }
    }
}

// =================================
//          VERTEX SHADER
// =================================
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
    vHeight = aPos.z;   // eftersom du kör Z-up

    gl_Position = MVP * vec4(aPos, 1.0);
}
)";


// =================================
//         FRAGMENT SHADER
// =================================
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

    vec3 c1 = vec3(0.0, 0.0, 0.5);  // mörkblå
    vec3 c2 = vec3(0.0, 0.0, 1.0);  // blå
    vec3 c3 = vec3(0.0, 1.0, 0.0);  // grön
    vec3 c4 = vec3(1.0, 1.0, 0.0);  // gul
    vec3 c5 = vec3(1.0, 0.0, 0.0);  // röd

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

    float diff = max(dot(n,l), 0.0);

    vec3 color;

    if (renderMode == 0)
    {
        // Solid mode
        color = baseColor;
    }
    else
    {
        // Heatmap mode
        float normalizedHeight = (vHeight - minZ) / (maxZ - minZ);
        normalizedHeight = clamp(normalizedHeight, 0.0, 1.0);
        color = heatmap(normalizedHeight);
    }

    vec3 ambient = 0.2 * color;
    vec3 diffuse = diff * color;

    FragColor = vec4(ambient + diffuse, 1.0);
}
)";

// =================================
//           FUNCTION
// =================================

//float func(float x, float y)
//{
//    return 15*std::sin(5*(sqrt(pow(x, 2) + pow(y, 2))))/(5*(sqrt(pow(x, 2) + pow(y, 2))));
//}

float func(float x, float y)
{
    return 3*cos(x)*sin(y);
}




// =================================
//             MAIN
// =================================
int main()
{
    // =================================
    //          INIT WINDOW 
    // =================================
    if (!glfwInit())
    {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window =
        glfwCreateWindow(800, 600, "Surfex (SURface EXplorer)", nullptr, nullptr);

    if (!window)
    {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, &renderMode);
    glfwSetKeyCallback(window, keyCallback);


    // =================================
    //           INIT GLAD 
    // =================================
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to init GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    // GL_LINE: Draw only mesh-lines, GL_FILL: Fill each triangle
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Shaders
    Shader surfaceShader(vertexShaderSource, fragmentShaderSource);


    // =================================
    //           INIT OBJECTS 
    // =================================

    float halfLength = 15.0f;

    // Camera
    Camera camera;
    camera.pitch  = 3.14f/8.0f;               // Up and down
    camera.yaw = 3.14f/4.0f;                  // Side to side
    camera.radius = 3*halfLength;              // Radius
    camera.target = glm::vec3(0.0f, 0.0f, 0.0f);


    // Mesh
    Mesh mesh = generateSurfaceMesh(
        func,
        -halfLength, halfLength,
        -halfLength, halfLength,
        1500, 1500
    );

    // Axis
    Axis axis;

    // Grid
    Grid grid(halfLength*1.5, 1.0f);

    // =================================
    //          VAO, VBO, EBO
    // =================================
    GLuint VAO, VBO, EBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Create array-object
    glBindVertexArray(VAO);

    // Send vertex-data to GPU (VBO)
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        mesh.vertices.size() * sizeof(float),
        mesh.vertices.data(),
        GL_STATIC_DRAW
    );

    // Send index-data to GPU (EBO)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        mesh.indices.size() * sizeof(unsigned int),
        mesh.indices.data(),
        GL_STATIC_DRAW
    );

    // Describe vertex-layout
    glVertexAttribPointer(
        0,              // location = 0
        3,              // 3 floats per vertex
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float),
        (void*)0
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT, 
        GL_FALSE,
        6 * sizeof(float),
        (void*)(3 * sizeof(float))
    );
    glEnableVertexAttribArray(1);

    // Unbind VAO?
    glBindVertexArray(0);







    // =================================
    //          RENDER LOOP
    // =================================

    float lastTime = static_cast<float>(glfwGetTime());

    while (!glfwWindowShouldClose(window))
    {
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;


        float rotateSpeed = glm::radians(90.0f) * deltaTime;
        float zoomSpeed   = halfLength * deltaTime;

        // ==== Input ====
        if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            camera.yaw -= rotateSpeed;

        if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            camera.yaw += rotateSpeed;


        if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            camera.pitch += rotateSpeed;

        if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            camera.pitch -= rotateSpeed;
        
        if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && camera.radius > 0)
            camera.radius -= zoomSpeed;
        
        if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.radius += zoomSpeed;
        
        if(glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        {
            camera.yaw = 0.0f;
            camera.pitch = 0.0f;
        }

        if(glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
        {
            camera.yaw = 3.1415f/2.0f;
            camera.pitch = 0;
        }

        if(glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        {
            camera.yaw = 0;
            camera.pitch = 3.1415f/2.0f;
        }

        

        camera.clampPitch();



        // 1. Clear
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        glViewport(0, 0, width, height);

        float aspect = static_cast<float>(width) / static_cast<float>(height);




        // ==== Calculate matrices ====
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.viewMatrix();
        glm::mat4 projection = camera.projectionMatrix(aspect);

        glm::mat4 MVP = projection * view * model;

        surfaceShader.use();

        surfaceShader.setFloat("minZ", mesh.minZ);
        surfaceShader.setFloat("maxZ", mesh.maxZ);

        surfaceShader.setMat4("model", model);

        glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, 1.0f, 2.0f));
        surfaceShader.setVec3("lightDir", lightDir);

        surfaceShader.setInt("renderMode", renderMode == RenderMode::Solid ? 0 : 1);
        glm::vec3 baseColor;

        switch(colorMode)
        {
            case ColorMode::Blue:   baseColor = {0.2f,0.6f,1.0f}; break;
            case ColorMode::Red:    baseColor = {1.0f,0.2f,0.2f}; break;
            case ColorMode::Green:  baseColor = {0.2f,1.0f,0.3f}; break;
            case ColorMode::Yellow: baseColor = {1.0f,0.9f,0.2f}; break;
        }

        surfaceShader.setVec3("baseColor", baseColor);

        surfaceShader.setMat4("MVP", MVP);

        // 5. Bind VAO
        glBindVertexArray(VAO);

        // 6. DRAW CALL
        glDrawElements(
            GL_TRIANGLES,
            static_cast<GLsizei>(mesh.indices.size()),
            GL_UNSIGNED_INT,
            0
        );
        if (showAxis)
        {
            axis.draw(MVP);
        }

        if (showGrid)
        {
            grid.draw(MVP);
        }

        // 7. Unbind VAO
        glBindVertexArray(0);

        // 8. Swap + poll
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Terminate window
    glfwTerminate();
    return 0;
}

