#include "Camera.h"

#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

/*
 * ============================================================
 * Coordinate convention:
 *   X = world forward/backward
 *   Y = world lateral
 *   Z = elevation (up)
 *   worldUp = (0, 0, 1)
 *
 * Yaw: azimuth angle in degrees, CCW from +X toward +Y.
 *   yaw = 0    → front faces +X
 *   yaw = 90   → front faces +Y
 *   yaw = -180 → front faces -X
 *
 * right = cross(front, worldUp) = (front.y, -front.x, 0)
 *   At yaw=0 (facing +X): right = (0, -1, 0) = -Y direction
 * Mouse right (xOffset > 0) rotates the camera clockwise in screen space,
 * therefore yaw is decreased below. ✓
 * ============================================================
 */

Camera::Camera()
    : position(-12.0f, -18.0f, 10.0f),
      yaw(56.0f),
      pitch(-22.0f),
      movementSpeed(10.0f),
      mouseSensitivity(0.10f),
      front(-1.0f, 0.0f, 0.0f),
      up(0.0f, 0.0f, 1.0f),
      right(0.0f, 1.0f, 0.0f)
{
    updateVectors();
}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(position, position + front, up);
}

void Camera::processKeyboard(int direction, float deltaTime)
{
    const float v = movementSpeed * deltaTime;
    switch (direction)
    {
        case 0: position += front * v; break;  // W forward
        case 1: position -= front * v; break;  // S backward
        case 2: position -= right * v; break;  // A strafe left
        case 3: position += right * v; break;  // D strafe right
        case 4: position -= up    * v; break;  // Q down
        case 5: position += up    * v; break;  // E up
        default: break;
    }
}

void Camera::processMouse(float xOffset, float yOffset)
{
    xOffset *= mouseSensitivity;
    yOffset *= mouseSensitivity;

    // GLFW supplies positive xOffset when the mouse moves right. With
    // yaw measured CCW from +X, increasing yaw rotates the view toward
    // the camera's right-hand side in this Z-up coordinate system.
    yaw   -= xOffset;
    pitch += yOffset;  // mouse up → pitch increases → view tilts upward ✓

    if (pitch >  89.0f) pitch =  89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    updateVectors();
}

glm::vec3 Camera::getPosition() const { return position; }

void Camera::updateVectors()
{
    glm::vec3 f;
    f.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
    f.y = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));
    f.z = std::sin(glm::radians(pitch));
    front = glm::normalize(f);

    // right = front × worldUp; with worldUp=(0,0,1) this gives (front.y, -front.x, 0)
    right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 0.0f, 1.0f)));

    // camera-local up (keeps horizon level while pitching)
    up = glm::normalize(glm::cross(right, front));
}