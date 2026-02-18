#include "include.hpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
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

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
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
    std::cout << "[debug] Shader ID = " << ourShader.ID << std::endl;
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

    bodyPart head(0.0f, 0.0f, 0.0f, BodyPartType::HEAD);
    bodyPart torso(0.5f, -1.5f, 0.0f, BodyPartType::TORSO);
    bodyPart torso2(0.5f, -2.5f, 0.0f, BodyPartType::TORSO);
    bodyPart torso3(-0.5f, -1.5f, 0.0f, BodyPartType::TORSO);
    bodyPart torso4(-0.5f, -2.5f, 0.0f, BodyPartType::TORSO);
    bodyPart leftArm1(-2.0f, -1.5f, 0.0f, BodyPartType::ARM);
    bodyPart leftArm2(-2.0f, -2.6f, 0.0f, BodyPartType::ARM);
    bodyPart rightArm1(2.0f, -1.5f, 0.0f, BodyPartType::ARM);
    bodyPart rightArm2(2.0f, -2.6f, 0.0f, BodyPartType::ARM);
    bodyPart leftLeg1(0.5f, -4.0f, 0.0f, BodyPartType::LEG);
    bodyPart leftLeg2(0.5f, -5.1f, 0.0f, BodyPartType::LEG);
    bodyPart rightLeg1(-0.6f, -4.0f, 0.0f, BodyPartType::LEG);
    bodyPart rightLeg2(-0.6f, -5.1f, 0.0f, BodyPartType::LEG);

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

    // world space positions of our cubes
    // glm::vec3 cubePositions[] = {
    //     // droite/gauche | haut/bas | avant/arrière
    //     // head
    //     glm::vec3( 0.0f,  0.0f,  0.0f),
    //     // body part 1
    //     glm::vec3( 0.5f,  -1.5f, 0.0f),
    //     // body part 2
    //     glm::vec3( 0.5f,  -2.0f, 0.0f),
    //     // body part 3
    //     glm::vec3( -0.5f,  -1.5f, 0.0f),
    //     // body part 4
    //     glm::vec3( -0.5f,  -2.0f, 0.0f),
    //     // left arm part 1
    //     glm::vec3(-2.0f, -1.5f, 0.0f),
    //     // left arm part 2
    //     glm::vec3(-2.0f, -2.5f, 0.0f),
    //     // right arm part 1
    //     glm::vec3(2.0f, -1.5f, 0.0f),
    //     // right arm part 2
    //     glm::vec3(2.0f, -2.5f, 0.0f),
    //     // left leg part 1
    //     glm::vec3(-0.6f, -3.5f, 0.0f),
    //     // left leg part 2
    //     glm::vec3(-0.6f, -4.5f, 0.0f),
    //     // right leg part 1
    //     glm::vec3(0.6f, -3.5f, 0.0f),
    //     // right leg part 2
    //     glm::vec3(0.6f, -4.5f, 0.0f),
    // };
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    {
        GLint bufSize = 0;
        glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufSize);
        std::cout << "[debug] VBO size = " << bufSize << " bytes\n";
        GLenum _err = glGetError();
        if (_err != GL_NO_ERROR) std::cout << "[debug] GL error after glBufferData: " << _err << std::endl;
    }

    // position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    std::cout << "[debug] VAO=" << VAO << " VBO=" << VBO << std::endl;


    // No textures: per-vertex colors are used instead


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

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
        // for (unsigned int i = 0; i < 13; i++)
        // {
        //     // calculate the model matrix for each object and pass it to shader before drawing
        //     glm::mat4 model = glm::mat4(1.0f);
        //     model = glm::translate(model, cubePositions[i]);
        //     // float angle = 20.0f * i;
        //     model = glm::rotate(model, glm::radians(0), glm::vec3(1.0f, 0.3f, 0.5f));
        //     ourShader.setMat4("model", model);

        //     glDrawArrays(GL_TRIANGLES, 0, 36);
        //     {
        //         GLenum _err = glGetError();
        //         if (_err != GL_NO_ERROR) std::cout << "GL error after draw: " << _err << std::endl;
        //     }
        // }

        // draw the head
        myBody.draw_head(ourShader);
        // draw the body
        myBody.draw_body(ourShader);
        // draw the arms
        myBody.draw_arm(ourShader);
        // draw the legs
        myBody.draw_leg(ourShader);

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
void processInput(GLFWwindow *window)
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

    (void)window;
    if (firstMouse)
    {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos); // reversed since y-coordinates go from bottom to top

    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    (void)window; (void)xoffset;
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}