#include "include.hpp"
#include "animation.hpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, bool &pressOneTime, Animator &animator);
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

    // gauche/droite | bas/haut | arriere/avant
    // up left
    glm::vec3 point = cubePosition + glm::vec3(-1.0f, 1.0f, 0.0f);
    attachmentPoints[point] = attachmentStates[0];

    // up back
    point = cubePosition + glm::vec3(0.0f, 1.0f, -1.0f);
    attachmentPoints[point] = attachmentStates[1];

    // up right
    point = cubePosition + glm::vec3(1.0f, 1.0f, 0.0f);
    attachmentPoints[point] = attachmentStates[2];

    // up front
    point = cubePosition + glm::vec3(0.0f, 1.0f, 1.0f);
    attachmentPoints[point] = attachmentStates[3];

    // left front
    point = cubePosition + glm::vec3(-1.0f, 0.0f, 1.0f);
    attachmentPoints[point] = attachmentStates[4];

    // left back
    point = cubePosition + glm::vec3(-1.0f, 0.0f, -1.0f);
    attachmentPoints[point] = attachmentStates[5];

    // right back
    point = cubePosition + glm::vec3(1.0f, 0.0f, -1.0f);
    attachmentPoints[point] = attachmentStates[6];

    // right front
    point = cubePosition + glm::vec3(1.0f, 0.0f, 1.0f);
    attachmentPoints[point] = attachmentStates[7];

    // down left
    point = cubePosition + glm::vec3(-1.0f, -1.0f, 0.0f);
    attachmentPoints[point] = attachmentStates[8];

    // down back
    point = cubePosition + glm::vec3(0.0f, -1.0f, -1.0f);
    attachmentPoints[point] = attachmentStates[9];

    // down right
    point = cubePosition + glm::vec3(1.0f, -1.0f, 0.0f);
    attachmentPoints[point] = attachmentStates[10];

    // down front
    point = cubePosition + glm::vec3(0.0f, -1.0f, 1.0f);
    attachmentPoints[point] = attachmentStates[11];

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

    // up, down, left, right, front, back
    // right/left | up/down | front/back
    const float HEAD_SCALE = 1.0f;
    const float TORSO_SCALE = 3.0f; // "corp = 3"
    const float ARM_SCALE = 2.0f;   // "bras = 2"
    const float LEG_SCALE = 2.0f;   // "jambe = 2"
    const float BASE = float(SIZE); // unit cube size
    // half extents of each part (world units)
    const float head_half = 0.5f * HEAD_SCALE * BASE;
    const float torso_half = 0.5f * TORSO_SCALE * BASE;
    const float arm_half = 0.5f * ARM_SCALE * BASE;
    const float leg_half = 0.5f * LEG_SCALE * BASE;

    // choose head center at y = 0.0
    glm::vec3 headCenter = {0.0f, 0.0f, 0.0f};

    // place torso directly below the head so they touch: headBottom == torsoTop
    glm::vec3 torsoCenter = {0.0f, headCenter.y - (head_half + torso_half), 0.0f};

    // arms attach at the vertical position near torso top (shoulder)
    float arm_attach_y = torsoCenter.y + torso_half - arm_half; // align upper-arm top with torso top

    // horizontal offset so arms sit beside the torso
    float arm_offset_x = torso_half + arm_half - (BASE * 0.5); // directly adjacent to torso

    glm::vec3 leftUpperArmCenter = {-arm_offset_x, arm_attach_y, 0.0f};
    glm::vec3 rightUpperArmCenter = {arm_offset_x, arm_attach_y, 0.0f};
    // lower arms positioned below upper arms so they touch
    glm::vec3 leftLowerArmCenter = {leftUpperArmCenter.x, leftUpperArmCenter.y - (arm_half + arm_half), 0.0f};
    glm::vec3 rightLowerArmCenter = {rightUpperArmCenter.x, rightUpperArmCenter.y - (arm_half + arm_half), 0.0f};

    // legs attach under the torso
    float leg_attach_x = 0.5f * BASE; // keep legs at +/-0.5 like original layout
    glm::vec3 leftThighCenter = {-leg_attach_x, torsoCenter.y - (torso_half + leg_half), 0.0f};
    glm::vec3 rightThighCenter = {leg_attach_x, torsoCenter.y - (torso_half + leg_half), 0.0f};
    glm::vec3 leftLowerLegCenter = {leftThighCenter.x, leftThighCenter.y - (leg_half + leg_half), 0.0f};
    glm::vec3 rightLowerLegCenter = {rightThighCenter.x, rightThighCenter.y - (leg_half + leg_half), 0.0f};

    // create parts with computed positions and sizes
    std::map<glm::vec3, int> attachmentPoints;

    attachmentPoints = setAtachementPoints(headCenter, {0, 0, 0, 0,
                                                        0, 0, 0, 0,
                                                        0, 0, 0, 2});
    bodyPart head(headCenter.x, headCenter.y, headCenter.z, BodyPartType::HEAD, attachmentPoints);
    head.setSize(glm::vec3(HEAD_SCALE * BASE, BASE, BASE));

    attachmentPoints = setAtachementPoints(torsoCenter, {2, 0, 2, 2,
                                                         0, 0, 0, 0,
                                                         0, 0, 0, 2});
    bodyPart torso(torsoCenter.x, torsoCenter.y, torsoCenter.z, BodyPartType::TORSO, attachmentPoints);
    torso.setSize(glm::vec3(TORSO_SCALE * BASE, TORSO_SCALE * BASE, BASE));

    // left arm
    attachmentPoints = setAtachementPoints(leftUpperArmCenter, {0, 0, 2, 0,
                                                                0, 0, 0, 0,
                                                                0, 0, 0, 2});
    bodyPart leftArm1(leftUpperArmCenter.x, leftUpperArmCenter.y, leftUpperArmCenter.z, BodyPartType::LEFT_UPPER_ARM, attachmentPoints);
    leftArm1.setSize(glm::vec3(BASE, ARM_SCALE * BASE, BASE));
    attachmentPoints = setAtachementPoints(leftLowerArmCenter, {0, 0, 0, 2,
                                                                0, 0, 0, 0,
                                                                0, 0, 0, 0});
    bodyPart leftArm2(leftLowerArmCenter.x, leftLowerArmCenter.y, leftLowerArmCenter.z, BodyPartType::LEFT_LOWER_ARM, attachmentPoints);
    leftArm2.setSize(glm::vec3(BASE, ARM_SCALE * BASE, BASE));

    // right arm
    attachmentPoints = setAtachementPoints(rightUpperArmCenter, {2, 0, 0, 0,
                                                                 0, 0, 0, 0,
                                                                 0, 0, 0, 2});
    bodyPart rightArm1(rightUpperArmCenter.x, rightUpperArmCenter.y, rightUpperArmCenter.z, BodyPartType::RIGHT_UPPER_ARM, attachmentPoints);
    rightArm1.setSize(glm::vec3(BASE, ARM_SCALE * BASE, BASE));
    attachmentPoints = setAtachementPoints(rightLowerArmCenter, {0, 0, 0, 2,
                                                                0, 0, 0, 0,
                                                                0, 0, 0, 0});
    bodyPart rightArm2(rightLowerArmCenter.x, rightLowerArmCenter.y, rightLowerArmCenter.z, BodyPartType::RIGHT_LOWER_ARM, attachmentPoints);
    rightArm2.setSize(glm::vec3(BASE, ARM_SCALE * BASE, BASE));

    // left leg (thigh + lower)
    attachmentPoints = setAtachementPoints(leftThighCenter, {0, 0, 0, 2,
                                                             0, 0, 0, 0,
                                                             0, 0, 0, 2});
    bodyPart leftLeg1(leftThighCenter.x, leftThighCenter.y, leftThighCenter.z, BodyPartType::LEFT_THIGH, attachmentPoints);
    leftLeg1.setSize(glm::vec3(BASE, LEG_SCALE * BASE, BASE));
    attachmentPoints = setAtachementPoints(leftLowerLegCenter, {0, 0, 0, 2,
                                                                0, 0, 0, 0,
                                                                0, 0, 0, 0});
    bodyPart leftLeg2(leftLowerLegCenter.x, leftLowerLegCenter.y, leftLowerLegCenter.z, BodyPartType::LEFT_LOWER_LEG, attachmentPoints);
    leftLeg2.setSize(glm::vec3(BASE, LEG_SCALE * BASE, BASE));

    // right leg
    attachmentPoints = setAtachementPoints(rightThighCenter, {0, 0, 0, 2,
                                                              0, 0, 0, 0,
                                                              0, 0, 0, 2});
    bodyPart rightLeg1(rightThighCenter.x, rightThighCenter.y, rightThighCenter.z, BodyPartType::RIGHT_THIGH, attachmentPoints);
    rightLeg1.setSize(glm::vec3(BASE, LEG_SCALE * BASE, BASE));
    attachmentPoints = setAtachementPoints(rightLowerLegCenter, {0, 0, 0, 2,
                                                                 0, 0, 0, 0,
                                                                 0, 0, 0, 0});
    bodyPart rightLeg2(rightLowerLegCenter.x, rightLowerLegCenter.y, rightLowerLegCenter.z, BodyPartType::RIGHT_LOWER_LEG, attachmentPoints);
    rightLeg2.setSize(glm::vec3(BASE, LEG_SCALE * BASE, BASE));

    // add to body
    myBody.addPart(head);
    myBody.addPart(torso);
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
    Animator animator;
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window, pressOneTime, animator);

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

        animator.update(deltaTime);
        animator.draw(ourShader, myBody);
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
void processInput(GLFWwindow *window, bool &pressOneTime, Animator &animator)
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

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::DOWN, deltaTime);

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

    static bool pressedAnimationKey = false;
    if (!pressedAnimationKey) { //TODO : SIMPLIFIER LE BORDEL KEY_PRESSED/RELEASE EN 3-4 LIGNES OUI C'EST POSSIBLE MAIS FLEMME ATM
        if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) {
            animator.setState(NONE);
            pressedAnimationKey = true;
        } else if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
            animator.setState(WAVING);
            pressedAnimationKey = true;
        } else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
            animator.setState(WALKING);
            pressedAnimationKey = true;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE)
        pressedAnimationKey = false;
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