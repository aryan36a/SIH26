#pragma once

#include <glm/glm.hpp>

class Camera {
public:
  Camera();

  glm::mat4 getViewMatrix() const;

  void processKeyboard(int direction, float deltaTime);

  void processMouse(float xOffset, float yOffset);

  glm::vec3 getPosition() const;

private:
  glm::vec3 position;

  float yaw;
  float pitch;

  float movementSpeed;
  float mouseSensitivity;

  glm::vec3 front;
  glm::vec3 up;
  glm::vec3 right;

  bool firstMouse;
  double lastMouseX;
  double lastMouseY;

  void updateVectors();
};