#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

Camera::Camera()
    : position(8.0f, 8.0f, 25.0f),
      yaw(-110.0f),
      pitch(-15.0f),
      movementSpeed(10.0f),
      mouseSensitivity(0.1f),
      front(0.0f, 0.0f, -1.0f),
      up(0.0f, 1.0f, 0.0f),
      right(1.0f, 0.0f, 0.0f)
{
    updateVectors();
}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(
        position,
        position + front,
        up
    );
}

void Camera::processKeyboard(
    int direction,
    float deltaTime)
{
    float velocity =
        movementSpeed * deltaTime;

    if (direction == 0)
    {
        position += front * velocity;
    }

    if (direction == 1)
    {
        position -= front * velocity;
    }

    if (direction == 2)
    {
        position -= right * velocity;
    }

    if (direction == 3)
    {
        position += right * velocity;
    }

    if (direction == 4)
    {
        position -= up * velocity;
    }

    if (direction == 5)
    {
        position += up * velocity;
    }
}

void Camera::processMouse(
    float xOffset,
    float yOffset)
{
    xOffset *= mouseSensitivity;
    yOffset *= mouseSensitivity;

    yaw += xOffset;
    pitch += yOffset;

    if (pitch > 89.0f)
    {
        pitch = 89.0f;
    }

    if (pitch < -89.0f)
    {
        pitch = -89.0f;
    }

    updateVectors();
}

glm::vec3 Camera::getPosition() const
{
    return position;
}

void Camera::updateVectors()
{
    glm::vec3 direction;

    direction.x =
        std::cos(glm::radians(yaw)) *
        std::cos(glm::radians(pitch));

    direction.y =
        std::sin(glm::radians(pitch));

    direction.z =
        std::sin(glm::radians(yaw)) *
        std::cos(glm::radians(pitch));

    front = glm::normalize(direction);

    right = glm::normalize(
        glm::cross(
            front,
            glm::vec3(0.0f, 1.0f, 0.0f)
        )
    );

    up = glm::normalize(
        glm::cross(right, front)
    );
}