#include <iostream>

#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"
#include "PointCloud.h"
#include "Renderer.h"
#include "SpatialGrid.h"

void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

int main() {
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW\n";
    return 1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(1280, 720, "SIH 26 - LiDAR Viewer", nullptr, nullptr);

  if (!window) {
    std::cerr << "Failed to create GLFW window\n";

    glfwTerminate();

    return 1;
  }

  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

  int version = gladLoadGL((GLADloadfunc)glfwGetProcAddress);

  if (version == 0) {
    std::cerr << "Failed to initialize GLAD\n";

    glfwDestroyWindow(window);
    glfwTerminate();

    return 1;
  }

  std::cout << "OpenGL initialized successfully\n";

  std::cout << "OpenGL version: " << glGetString(GL_VERSION) << '\n';
  glEnable(GL_DEPTH_TEST);

  PointCloud cloud;

  if (!cloud.loadBIN("data/test.bin")) {
    std::cerr << "Failed to load point cloud\n";

    glfwDestroyWindow(window);
    glfwTerminate();

    return 1;
  }

  std::cout << "Point cloud loaded successfully\n";

  std::cout << "Points: " << cloud.size() << '\n';

  std::cout << "X range: " << cloud.getMinX() << " -> " << cloud.getMaxX()
            << '\n';

  std::cout << "Y range: " << cloud.getMinY() << " -> " << cloud.getMaxY()
            << '\n';

  std::cout << "Z range: " << cloud.getMinZ() << " -> " << cloud.getMaxZ()
            << '\n';

  std::cout << "Intensity range: " << cloud.getMinIntensity() << " -> "
            << cloud.getMaxIntensity() << '\n';
  SpatialGrid grid(cloud.getMinX(), cloud.getMaxX(), cloud.getMinY(),
                   cloud.getMaxY(), 1.0f);

  grid.build(cloud);

  std::cout << "Grid width: " << grid.getWidth() << '\n';

  std::cout << "Grid height: " << grid.getHeight() << '\n';
  std::cout << "Cell (0, 0) has data: " << grid.hasData(0, 0) << '\n';

  std::cout << "Cell (0, 0) elevation: " << grid.getElevation(0, 0) << '\n';

  std::cout << "Cell (0, 0) intensity: " << grid.getIntensity(0, 0) << '\n';

  std::cout << "Cell (1, 1) has data: " << grid.hasData(1, 1) << '\n';

  Renderer renderer;
  Camera camera;

  if (!renderer.uploadPointCloud(cloud)) {
    std::cerr << "Failed to upload point cloud\n";

    glfwDestroyWindow(window);
    glfwTerminate();

    return 1;
  }
  if (!renderer.uploadGrid(grid)) {
    std::cerr << "Failed to upload grid\n";

    glfwDestroyWindow(window);
    glfwTerminate();

    return 1;
  }
  glDisable(GL_PROGRAM_POINT_SIZE);
  glPointSize(10.0f);
  float lastFrame = 0.0f;
  double lastMouseX = 640.0;
  double lastMouseY = 360.0;
  bool firstMouse = true;
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  bool mouseCaptured = true;

  while (!glfwWindowShouldClose(window)) {
    float currentFrame = static_cast<float>(glfwGetTime());

    float deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    static bool escapeWasPressed = false;

    bool escapePressed = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;

    if (escapePressed && !escapeWasPressed) {
      mouseCaptured = !mouseCaptured;

      if (mouseCaptured) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      }
    }

    escapeWasPressed = escapePressed;

    // Keyboard

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
      camera.processKeyboard(0, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
      camera.processKeyboard(1, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
      camera.processKeyboard(2, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
      camera.processKeyboard(3, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
      camera.processKeyboard(4, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
      camera.processKeyboard(5, deltaTime);

    // Mouse

    // Mouse

    if (mouseCaptured) {
      double mouseX;
      double mouseY;

      glfwGetCursorPos(window, &mouseX, &mouseY);

      if (firstMouse) {
        lastMouseX = mouseX;
        lastMouseY = mouseY;

        firstMouse = false;
      }

      auto xOffset = static_cast<float>(mouseX - lastMouseX);

      auto yOffset = static_cast<float>(lastMouseY - mouseY);

      lastMouseX = mouseX;
      lastMouseY = mouseY;

      camera.processMouse(xOffset, yOffset);
    } else {
      firstMouse = true;
    }
    // Matrices

    // Matrices

    glm::mat4 view = camera.getViewMatrix();

    int width;
    int height;

    glfwGetFramebufferSize(window, &width, &height);

    glm::mat4 projection = glm::perspective(
        glm::radians(60.0f),
        static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f);

    // Render

    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderer.render(grid, view, projection);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}