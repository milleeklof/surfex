#include"Camera.h"
#include<cmath>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>


glm::mat4 Camera::viewMatrix() const
{
    glm::vec3 eye;
    eye.x = target.x + radius * cos(pitch) * cos(yaw);
    eye.z = target.z + radius * sin(pitch);
    eye.y = target.y + radius * cos(pitch) * sin(yaw);

    return glm::lookAt(eye, target, glm::vec3(0,0,1));


}

glm::mat4 Camera::projectionMatrix(float aspect) const
{
    float fov = glm::radians(45.0f); // Converts degrees into radians
    float nearPlane = 0.1f;
    float farPlane  = 100.0f;

    return glm::perspective(fov, aspect, nearPlane, farPlane);
}

void Camera::clampPitch()
{
    if (pitch >  maxPitch) pitch =  maxPitch;
    if (pitch < -maxPitch) pitch = -maxPitch;
}
