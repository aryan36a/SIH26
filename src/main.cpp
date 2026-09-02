#include <iostream>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glad/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "AdaptiveSpatialGrid.h"
#include "Camera.h"
#include "PointCloud.h"
#include "Renderer.h"
#include "ResolutionProfile.h"
#include "SpatialGrid.h"

void framebufferSizeCallback(
    GLFWwindow* window,
    int width,
    int height)
{
    glViewport(
        0,
        0,
        width,
        height
    );
}

int main()
{
    if (!glfwInit())
    {
        std::cerr
            << "Failed to initialize GLFW\n";

        return 1;
    }

    glfwWindowHint(
        GLFW_CONTEXT_VERSION_MAJOR,
        3
    );

    glfwWindowHint(
        GLFW_CONTEXT_VERSION_MINOR,
        3
    );

    glfwWindowHint(
        GLFW_OPENGL_PROFILE,
        GLFW_OPENGL_CORE_PROFILE
    );

    GLFWwindow* window =
        glfwCreateWindow(
            1280,
            720,
            "SIH 26 - Adaptive 2.5D LiDAR Viewer",
            nullptr,
            nullptr
        );

    if (!window)
    {
        std::cerr
            << "Failed to create GLFW window\n";

        glfwTerminate();

        return 1;
    }

    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(
        window,
        framebufferSizeCallback
    );

    int version =
        gladLoadGL(
            (GLADloadfunc)glfwGetProcAddress
        );

    if (version == 0)
    {
        std::cerr
            << "Failed to initialize GLAD\n";

        glfwDestroyWindow(window);
        glfwTerminate();

        return 1;
    }

    std::cout
        << "OpenGL initialized successfully\n";

    std::cout
        << "OpenGL version: "
        << glGetString(GL_VERSION)
        << '\n';

    glEnable(
        GL_DEPTH_TEST
    );

    /*
     * --------------------------------------------------
     * Load LiDAR
     * --------------------------------------------------
     */

    PointCloud cloud;

    if (!cloud.loadBIN(
            "data/test.bin"))
    {
        std::cerr
            << "Failed to load point cloud\n";

        glfwDestroyWindow(window);
        glfwTerminate();

        return 1;
    }

    std::cout
        << "Point cloud loaded successfully\n";

    std::cout
        << "Points: "
        << cloud.size()
        << '\n';

    std::cout
        << "X range: "
        << cloud.getMinX()
        << " -> "
        << cloud.getMaxX()
        << '\n';

    std::cout
        << "Y range: "
        << cloud.getMinY()
        << " -> "
        << cloud.getMaxY()
        << '\n';

    std::cout
        << "Z range: "
        << cloud.getMinZ()
        << " -> "
        << cloud.getMaxZ()
        << '\n';

    std::cout
        << "Intensity range: "
        << cloud.getMinIntensity()
        << " -> "
        << cloud.getMaxIntensity()
        << '\n';

    /*
     * --------------------------------------------------
     * Uniform baseline
     * --------------------------------------------------
     */

    SpatialGrid grid(
        cloud.getMinX(),
        cloud.getMaxX(),
        cloud.getMinY(),
        cloud.getMaxY(),
        1.0f
    );

    grid.build(cloud);

    std::cout
        << "Uniform grid width: "
        << grid.getWidth()
        << '\n';

    std::cout
        << "Uniform grid height: "
        << grid.getHeight()
        << '\n';

    /*
     * --------------------------------------------------
     * Adaptive grid
     * --------------------------------------------------
     */

    ResolutionProfile resolutionProfile;

    AdaptiveSpatialGrid adaptiveGrid(
        cloud.getMinX(),
        cloud.getMaxX(),
        cloud.getMinY(),
        cloud.getMaxY(),
        resolutionProfile
    );

    adaptiveGrid.build(
        cloud
    );

    std::cout
        << "Adaptive cell count: "
        << adaptiveGrid.getCellCount()
        << '\n';

    /*
     * --------------------------------------------------
     * Renderer
     * --------------------------------------------------
     */

    Renderer renderer;

    if (!renderer.uploadPointCloud(
            cloud))
    {
        std::cerr
            << "Failed to upload point cloud\n";

        glfwDestroyWindow(window);
        glfwTerminate();

        return 1;
    }

    if (!renderer.uploadGrid(
            grid))
    {
        std::cerr
            << "Failed to upload uniform grid\n";

        glfwDestroyWindow(window);
        glfwTerminate();

        return 1;
    }

    if (!renderer.uploadAdaptiveGrid(
            adaptiveGrid))
    {
        std::cerr
            << "Failed to upload adaptive grid\n";

        glfwDestroyWindow(window);
        glfwTerminate();

        return 1;
    }

    /*
     * --------------------------------------------------
     * Camera
     * --------------------------------------------------
     */

    Camera camera;

    float lastFrame = 0.0f;

    double lastMouseX = 640.0;
    double lastMouseY = 360.0;

    bool firstMouse = true;

    bool mouseCaptured = true;

    glfwSetInputMode(
        window,
        GLFW_CURSOR,
        GLFW_CURSOR_DISABLED
    );

    /*
     * --------------------------------------------------
     * Main loop
     * --------------------------------------------------
     */

    while (!glfwWindowShouldClose(window))
    {
        const float currentFrame =
            static_cast<float>(
                glfwGetTime()
            );

        const float deltaTime =
            currentFrame - lastFrame;

        lastFrame =
            currentFrame;

        /*
         * --------------------------------------------------
         * Escape toggles mouse capture
         * --------------------------------------------------
         */

        static bool escapeWasPressed = false;

        const bool escapePressed =
            glfwGetKey(
                window,
                GLFW_KEY_ESCAPE
            ) == GLFW_PRESS;

        if (escapePressed &&
            !escapeWasPressed)
        {
            mouseCaptured =
                !mouseCaptured;

            if (mouseCaptured)
            {
                glfwSetInputMode(
                    window,
                    GLFW_CURSOR,
                    GLFW_CURSOR_DISABLED
                );
            }
            else
            {
                glfwSetInputMode(
                    window,
                    GLFW_CURSOR,
                    GLFW_CURSOR_NORMAL
                );
            }
        }

        escapeWasPressed =
            escapePressed;

        /*
         * --------------------------------------------------
         * Keyboard movement
         * --------------------------------------------------
         */

        if (glfwGetKey(
                window,
                GLFW_KEY_W) == GLFW_PRESS)
        {
            camera.processKeyboard(
                0,
                deltaTime
            );
        }

        if (glfwGetKey(
                window,
                GLFW_KEY_S) == GLFW_PRESS)
        {
            camera.processKeyboard(
                1,
                deltaTime
            );
        }

        if (glfwGetKey(
                window,
                GLFW_KEY_A) == GLFW_PRESS)
        {
            camera.processKeyboard(
                2,
                deltaTime
            );
        }

        if (glfwGetKey(
                window,
                GLFW_KEY_D) == GLFW_PRESS)
        {
            camera.processKeyboard(
                3,
                deltaTime
            );
        }

        if (glfwGetKey(
                window,
                GLFW_KEY_Q) == GLFW_PRESS)
        {
            camera.processKeyboard(
                4,
                deltaTime
            );
        }

        if (glfwGetKey(
                window,
                GLFW_KEY_E) == GLFW_PRESS)
        {
            camera.processKeyboard(
                5,
                deltaTime
            );
        }

        /*
         * --------------------------------------------------
         * Mouse
         * --------------------------------------------------
         */

        if (mouseCaptured)
        {
            double mouseX;
            double mouseY;

            glfwGetCursorPos(
                window,
                &mouseX,
                &mouseY
            );

            if (firstMouse)
            {
                lastMouseX =
                    mouseX;

                lastMouseY =
                    mouseY;

                firstMouse =
                    false;
            }

            const float xOffset =
                static_cast<float>(
                    mouseX - lastMouseX
                );

            const float yOffset =
                static_cast<float>(
                    lastMouseY - mouseY
                );

            lastMouseX =
                mouseX;

            lastMouseY =
                mouseY;

            camera.processMouse(
                xOffset,
                yOffset
            );
        }
        else
        {
            firstMouse = true;
        }

        /*
         * --------------------------------------------------
         * Matrices
         * --------------------------------------------------
         */

        const glm::mat4 view =
            camera.getViewMatrix();

        int width;
        int height;

        glfwGetFramebufferSize(
            window,
            &width,
            &height
        );

        const glm::mat4 projection =
            glm::perspective(
                glm::radians(60.0f),
                static_cast<float>(width) /
                    static_cast<float>(height),
                0.1f,
                200.0f
            );
/*
 * --------------------------------------------------
 * Render
 * --------------------------------------------------
 */

glClearColor(
    0.03f,
    0.03f,
    0.03f,
    1.0f
);

glClear(
    GL_COLOR_BUFFER_BIT |
    GL_DEPTH_BUFFER_BIT
);

/*
 * --------------------------------------------------
 * Visualization
 * --------------------------------------------------
 *
 * 0 = raw LiDAR point cloud
 * 1 = adaptive 2.5D mesh
 */

static int renderMode = 0;

/*
 * --------------------------------------------------
 * Mode switching
 * --------------------------------------------------
 */

static bool key1WasPressed = false;
static bool key2WasPressed = false;

const bool key1Pressed =
    glfwGetKey(
        window,
        GLFW_KEY_1
    ) == GLFW_PRESS;

const bool key2Pressed =
    glfwGetKey(
        window,
        GLFW_KEY_2
    ) == GLFW_PRESS;

if (key1Pressed &&
    !key1WasPressed)
{
    renderMode = 0;

    std::cout
        << "Render mode: Raw LiDAR\n";
}

if (key2Pressed &&
    !key2WasPressed)
{
    renderMode = 1;

    std::cout
        << "Render mode: Adaptive 2.5D\n";
}

key1WasPressed =
    key1Pressed;

key2WasPressed =
    key2Pressed;

/*
 * --------------------------------------------------
 * Render selected representation
 * --------------------------------------------------
 */

if (renderMode == 0)
{
    renderer.render(
        cloud,
        view,
        projection
    );
}
else
{
    renderer.renderAdaptive(
        view,
        projection
    );
}

glfwSwapBuffers(
    window
);

glfwPollEvents();
    }

    glfwDestroyWindow(
        window
    );

    glfwTerminate();

    return 0;
}