#pragma once
#include <glm/glm.hpp>

// Camera is supposed to alter the mesh object to look as if it is viewed from another direction or radius. 
// Camera is orbit-based, meaning that it exists on a sphere of jaw, pitch, radius. 
struct Camera
{
    float pitch, yaw, radius;

    glm::vec3 target;

    static constexpr float maxPitch = glm::radians(89.0f);

    glm::mat4 viewMatrix() const;
    glm::mat4 projectionMatrix(float aspect) const;

    void clampPitch();

};