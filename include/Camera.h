#pragma once

#include <glm/glm.hpp>

class Camera
{
public:
    Camera();

    glm::mat4 getViewMatrix() const;

    // direction: 0=forward, 1=back, 2=left, 3=right, 4=down, 5=up
    void processKeyboard(int direction, float deltaTime);

    // xOffset: positive when mouse moves right
    // yOffset: positive when mouse moves up
    void processMouse(float xOffset, float yOffset);

    glm::vec3 getPosition() const;

private:
    glm::vec3 position;

    float yaw;    // azimuth degrees, CCW from +X
    float pitch;  // elevation degrees

    float movementSpeed;
    float mouseSensitivity;

    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;

    void updateVectors();
};