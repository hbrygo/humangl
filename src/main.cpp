#include "include.hpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, bool &pressOneTime);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera globals (defined after SCR_* so they can reference them)
Camera camera;
float lastX = 0.0f;
float lastY = 0.0f;
bool firstMouse = true;
bool mouseOn = false;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

std::map<glm::vec3, int> setAtachementPoints(const glm::vec3& cubePosition, std::vector<int> attachmentStates)
{
    std::map<glm::vec3, int> attachmentPoints;

    // up
    glm::vec3 point = cubePosition + glm::vec3(0.0f, 0.5f, 0.0f);
    attachmentPoints[point] = attachmentStates[0];

    // down
    point = cubePosition + glm::vec3(0.0f, -0.5f, 0.0f);
    attachmentPoints[point] = attachmentStates[1];

    // left
    point = cubePosition + glm::vec3(-0.5f, -0.0f, 0.0f);
    attachmentPoints[point] = attachmentStates[2];

    // right
    point = cubePosition + glm::vec3(0.5f, -0.0f, 0.0f);
    attachmentPoints[point] = attachmentStates[3];

    // front
    point = cubePosition + glm::vec3(0.0f, -0.0f, 0.5f);
    attachmentPoints[point] = attachmentStates[4];

    // back
    point = cubePosition + glm::vec3(0.0f, -0.0f, -0.5f);
    attachmentPoints[point] = attachmentStates[5];

    return attachmentPoints;
}

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Lderidde", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    // set initial viewport using framebuffer size (handles HiDPI / Retina)
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);
    // initialize last mouse pos to center of framebuffer
    lastX = fbWidth / 2.0f;
    lastY = fbHeight / 2.0f;

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader("6.3.coordinate_systems.vs", "6.3.coordinate_systems.fs");
    {
        GLenum _err = glGetError();
        if (_err != GL_NO_ERROR) std::cout << "[debug] GL error after shader creation: " << _err << std::endl;
    }

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        // front (red)
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
        // back (green)
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
        // left (blue)
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
        // right (yellow)
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
        // top (magenta)
        -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
        // bottom (cyan)
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f
    };

    body myBody;

    std::map<glm::vec3, int> attachmentPoints;

    // up, down, left, right, front, back
    // right/left | up/down | front/back
    attachmentPoints = setAtachementPoints({0.0f, 0.0f, 0.0f}, {0, 2, 0, 0, 0, 0});
    bodyPart head(0.0f, 0.0f, 0.0f, BodyPartType::HEAD, attachmentPoints);
    
    // right up torso
    attachmentPoints = setAtachementPoints({0.5f, -1.0f, 0.0f}, {0, 1, 1, 2, 0, 0});
    bodyPart torso(0.5f, -1.0f, 0.0f, BodyPartType::TORSO, attachmentPoints);
    
    // right down torso
    attachmentPoints = setAtachementPoints({0.5f, -2.0f, 0.0f}, {1, 2, 1, 0, 0, 0});
    bodyPart torso2(0.5f, -2.0f, 0.0f, BodyPartType::TORSO, attachmentPoints);
    
    // left up torso
    attachmentPoints = setAtachementPoints({-0.5f, -1.0f, 0.0f}, {0, 1, 2, 1, 0, 0});
    bodyPart torso3(-0.5f, -1.0f, 0.0f, BodyPartType::TORSO, attachmentPoints);

    // left down torso
    attachmentPoints = setAtachementPoints({-0.5f, -2.0f, 0.0f}, {1, 2, 0, 1, 0, 0});
    bodyPart torso4(-0.5f, -2.0f, 0.0f, BodyPartType::TORSO, attachmentPoints);
    
    // up left arm
    attachmentPoints = setAtachementPoints({-1.5f, -1.0f, 0.0f}, {0, 2, 0, 2, 0, 0});
    bodyPart leftArm1(-1.5f, -1.0f, 0.0f, BodyPartType::UPPER_ARM, attachmentPoints);
    
    // down left arm
    attachmentPoints = setAtachementPoints({-1.5f, -2.0f, 0.0f}, {2, 0, 0, 0, 0, 0});
    bodyPart leftArm2(-1.5f, -2.0f, 0.0f, BodyPartType::LOWER_ARM, attachmentPoints);
    
    // up right arm
    attachmentPoints = setAtachementPoints({1.5f, -1.0f, 0.0f}, {0, 2, 2, 0, 0, 0});
    bodyPart rightArm1(1.5f, -1.0f, 0.0f, BodyPartType::UPPER_ARM, attachmentPoints);
    
    // down right arm
    attachmentPoints = setAtachementPoints({1.5f, -2.0f, 0.0f}, {2, 0, 0, 0, 0, 0});
    bodyPart rightArm2(1.5f, -2.0f, 0.0f, BodyPartType::LOWER_ARM, attachmentPoints);

    // up left leg
    attachmentPoints = setAtachementPoints({-0.5f, -3.0f, 0.0f}, {2, 2, 0, 0, 0, 0});
    bodyPart leftLeg1(-0.5f, -3.0f, 0.0f, BodyPartType::THIGH, attachmentPoints);
    
    // down left leg
    attachmentPoints = setAtachementPoints({-0.5f, -3.0f, 0.0f}, {2, 0, 0, 0, 0, 0});
    bodyPart leftLeg2(-0.5f, -4.0f, 0.0f, BodyPartType::LOWER_PART, attachmentPoints);
    
    // up right leg
    attachmentPoints = setAtachementPoints({-0.5f, -3.0f, 0.0f}, {2, 2, 0, 0, 0, 0});
    bodyPart rightLeg1(0.5f, -3.0f, 0.0f, BodyPartType::THIGH, attachmentPoints);
    
    // down right leg
    attachmentPoints = setAtachementPoints({-0.5f, -3.0f, 0.0f}, {2, 0, 0, 0, 0, 0});
    bodyPart rightLeg2(0.5f, -4.0f, 0.0f, BodyPartType::LOWER_PART, attachmentPoints);

    // glm::vec3 cubePos = {0.0f, 0.0f, 2.0f};
    // const float CUBE_HALF = -0.50f;
    // glm::vec3 topCenter = cubePos + glm::vec3(0.0f, CUBE_HALF, 0.0f);
    // attachmentPoints = {{topCenter, true}};
    // bodyPart test(cubePos.x, cubePos.y, cubePos.z, BodyPartType::WALL, attachmentPoints);

    // myBody.addPart(test);

    // --------------------------------------------   DRAW WALL   --------------------------------------------
    
    
    // generate a configurable wall grid instead of many manual variables
    // Change WALL_ROWS and WALL_COLS to control how many wall cubes are created.
    // Note: large values (e.g. 100x100 = 10k cubes) can impact performance.
    // const int WALL_ROWS = 30; // number of rows (vertical)
    // const int WALL_COLS = 30; // number of columns (horizontal)
    // const float WALL_START_X = 15.0f;   // starting X coordinate
    // const float WALL_START_Y = 15.0f;  // starting Y coordinate
    // const float WALL_SPACING_X = -1.0f; // spacing between columns (negative to go left)
    // const float WALL_SPACING_Y = -1.0f; // spacing between rows (downwards)
    // const float WALL_Z = -15.0f;
    // std::vector<bodyPart> walls;
    // walls.reserve(static_cast<size_t>(WALL_ROWS) * static_cast<size_t>(WALL_COLS));
    // for (int r = 0; r < WALL_ROWS; ++r) {
    //     for (int c = 0; c < WALL_COLS; ++c) {
    //         float x = WALL_START_X + c * WALL_SPACING_X;
    //         float y = WALL_START_Y + r * WALL_SPACING_Y;
    //         walls.emplace_back(x, y, WALL_Z, BodyPartType::WALL);
    //     }
    // }
    // add generated wall parts
    // for (const auto& w : walls) myBody.addPart(w);

    myBody.addPart(head);
    myBody.addPart(torso);
    myBody.addPart(torso2);
    myBody.addPart(torso3);
    myBody.addPart(torso4);
    myBody.addPart(leftArm1);
    myBody.addPart(leftArm2);
    myBody.addPart(rightArm1);
    myBody.addPart(rightArm2);
    myBody.addPart(leftLeg1);
    myBody.addPart(leftLeg2);
    myBody.addPart(rightLeg1);
    myBody.addPart(rightLeg2);

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    {
        GLint bufSize = 0;
        glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufSize);
        GLenum _err = glGetError();
        if (_err != GL_NO_ERROR) std::cout << "[debug] GL error after glBufferData: " << _err << std::endl;
    }

    // position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // render loop
    // -----------
    bool pressOneTime = false;
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window, pressOneTime);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

    // no textures to bind

        // activate shader
        ourShader.use();

        // create transformations
        glm::mat4 projection    = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        // pass transformation matrices to the shader
        ourShader.setMat4("projection", projection); // note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
        ourShader.setMat4("view", view);

        // render boxes
        glBindVertexArray(VAO);
        
        myBody.draw_head(ourShader);
        myBody.draw_arm(ourShader);
        myBody.draw_body(ourShader);
        myBody.draw_leg(ourShader);
        // myBody.draw_wall(ourShader);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window, bool &pressOneTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::RIGHT, deltaTime);

    // pas touche a ca, ca marche
    if (!pressOneTime) {
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
            // std::cout << "C press" << std::endl;
            mouseOn = (mouseOn == false) ? true : false;
            if (mouseOn)
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            else
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            pressOneTime = true;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE)
        pressOneTime = false;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    (void)window;
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    // std::cout << "Mouse position: (" << xpos << ", " << ypos << ")" << std::endl;
    if (!mouseOn)
        return ;
    (void)window;
    if (firstMouse)
    {
        // std::cout << "Mouse position: (" << xpos << ", " << ypos << ")" << std::endl;
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }
    
    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos); // reversed since y-coordinates go from bottom to top
    
    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);
    
    camera.ProcessMouseMovement(xoffset, yoffset);
    // if (mouseOn)
    //     glfwSetCursorPos(window, 400, 300);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    (void)window; (void)xoffset;
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}