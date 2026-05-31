#pragma once
#include <glm/glm.hpp>

// Orbit camera around the surface using pitch, yaw, and radius.
struct Camera
{
    float pitch, yaw, radius;

    glm::vec3 target;

    static constexpr float maxPitch = 1.5533431f; // 89 degrees in radians

    glm::mat4 viewMatrix() const;
    glm::mat4 projectionMatrix(float aspect) const;

    void clampPitch();

};
