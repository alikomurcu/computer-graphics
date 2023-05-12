#include "main.h"

unsigned int framebuffer;
unsigned int cubeBuffer;
unsigned int rbo;

int gWidth, gHeight;

glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix;
glm::mat4 modelingMatrix;

// models details
Object *cybertruck, *cbWindows, *cbTires, *ground, *armadillo, *teapot;
Skybox* skybox;

// reflectance coeffients
const float carFactor = 0.57f;
const float windowFactor = 0.95f;

//DynamicSkybox *dynamicSkybox;

// shaders
Shader *groundShader;
unsigned int gVAO;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)WIDTH / 2.0;
float lastY = (float)HEIGHT / 2.0;
bool firstMouse = true;
glm::vec3 cameraOffset(0.0f, 2.0f, -7.0f);

// Ground texture
GLuint groundTextureID;

void initVBO()
{
    armadillo->InitObject();
    teapot->InitObject();
    cybertruck->InitObject();
    cbWindows->InitObject();
    cbTires->InitObject();
    skybox->loadCubemap();
}

void handle_texture()
{
    groundTextureID = loadTexture("textures/ground_texture_sand.jpg");
    groundShader = new Shader("shaders/groundtexvs.glsl", "shaders/groundtexfs.glsl");
    float vertices[] = {
            // positions          // colors           // texture coords
            0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
            0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
            -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
            -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left
    };
    unsigned int indices[] = {
            0, 1, 3, // first triangle
            1, 2, 3  // second triangle
    };
    unsigned int VBO, EBO;
    glGenVertexArrays(1, &gVAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(gVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    groundShader->setInt("texture1", 0);
    glBindVertexArray(0);
}

void handle_framebuffer()
{
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glGenTextures(1, &cubeBuffer);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeBuffer);
    for (int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, WIDTH, WIDTH, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // attach it to currently bound FBO
    for (int i = 0; i < 6; ++i)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubeBuffer, 0);
    }
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, WIDTH);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // attach it to currently bound FBO
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    // check framebuffer status
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n";
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void init()
{
    glEnable(GL_DEPTH_TEST);
    armadillo = new Object("obj/armadillo.obj");
    armadillo->transform->ScaleUp(3.f, 3.f, 3.f);
    armadillo->transform->Position += glm::vec3(0.f, 2.f, 0.f);
    armadillo->initShader("shaders/armadillovs.glsl", "shaders/armadillofs.glsl");

    teapot = new Object("obj/teapot.obj");
    teapot->transform->ScaleUp(2.f, 2.f, 2.f);
    teapot->transform->Position = glm::vec3(25.f, 2.f, -10.f);
    teapot->initShader("shaders/teapotvs.glsl", "shaders/teapotfs.glsl");

    cybertruck = new Object("obj/cybertruck/cybertruck_body.obj");
    cybertruck->initShader("shaders/reflectvs.glsl", "shaders/reflectfs.glsl");

    cbWindows = new Object("obj/cybertruck/cybertruck_windows.obj");
    cbWindows->initShader("shaders/reflectvs.glsl", "shaders/reflectfs.glsl");

    cbTires = new Object("obj/cybertruck/cybertruck_tires.obj");
    cbTires->initShader("shaders/tirevs.glsl", "shaders/tirefs.glsl");

    ground = new Object("obj/ground.obj");
    ground->initShader("shaders/groundtexvs.glsl", "shaders/groundtexfs.glsl");

    skybox = new Skybox();
    skybox->initShader("shaders/skyvs.glsl", "shaders/skyfs.glsl");
    skybox->shader->use();
    skybox->shader->setInt("skybox", 0);

//    dynamicSkybox = new DynamicSkybox();
//    dynamicSkybox->initShader("shaders/skyvs.glsl", "shaders/skyfs.glsl");
//    dynamicSkybox->shader->use();
//    dynamicSkybox->shader->setInt("skybox", 0);
//    dynamicSkybox->initFrameBuffer();

    handle_framebuffer();

    initVBO();

    // texture
    handle_texture();
}

void drawFrameBuffer()
{
    glm::vec3 carPos = cybertruck->transform->Position;
//    glViewport(0, 0, WIDTH, WIDTH);

    projectionMatrix = glm::perspective(glm::radians(90.f), 1.f, 10.f, 100.0f);

    for (int i = 0; i < faces.size(); ++i) {
        int face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;


        if (face == GL_TEXTURE_CUBE_MAP_POSITIVE_X)
        {
            viewingMatrix = glm::lookAt(carPos, carPos + glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f));
        }
        else if(face == GL_TEXTURE_CUBE_MAP_NEGATIVE_X)
        {
            viewingMatrix = glm::lookAt(carPos, carPos + glm::vec3(-1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f));
        }
        else if(face == GL_TEXTURE_CUBE_MAP_POSITIVE_Y)
        {
            viewingMatrix = glm::lookAt(carPos, carPos + glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
        }
        else if(face == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y)
        {
            viewingMatrix = glm::lookAt(carPos, carPos + glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, 0.f, -1.f));
        }
        else if(face == GL_TEXTURE_CUBE_MAP_POSITIVE_Z)
        {
            viewingMatrix = glm::lookAt(carPos, carPos + glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, -1.f, 0.f));
        }
        else if(face == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
        {
            viewingMatrix = glm::lookAt(carPos, carPos + glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, -1.f, 0.f));
        }


        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        //armadillo transformation
        armadillo->transform->SetModelMatrix();
        modelingMatrix = armadillo->transform->ModelMatrix;
        // Set the viewing matrix and the projection matrix
        // Set the active program and the values of its uniform variables
        armadillo->shader->use();
        armadillo->shader->setMat4("projectionMatrix", projectionMatrix);
        armadillo->shader->setMat4("viewingMatrix", viewingMatrix);
        armadillo->shader->setMat4("modelingMatrix", modelingMatrix);
        armadillo->shader->setVec3("eyePos", camera.transform->Position);
        // Draw the scene
        armadillo->draw();

        //teapot transformation
        teapot->transform->SetModelMatrix();
        modelingMatrix = teapot->transform->ModelMatrix;
        // Set the active program and the values of its uniform variables
        teapot->shader->use();
        teapot->shader->setMat4("projectionMatrix", projectionMatrix);
        teapot->shader->setMat4("viewingMatrix", viewingMatrix);
        teapot->shader->setMat4("modelingMatrix", modelingMatrix);
        teapot->shader->setVec3("eyePos", camera.transform->Position);
        // Draw the scene
        teapot->draw();


        // textures
        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, groundTextureID);
        // texture transformation
        glm::mat4 matRy = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
        glm::mat4 matS = glm::scale(glm::mat4(1.0f), glm::vec3(100.0f, 1.0f, 100.0f));
        glm::mat4 matT = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.0f, 0.0f));
        modelingMatrix = matT * matS * matRy;
        // render container
        groundShader->use();
        groundShader->setMat4("projectionMatrix", projectionMatrix);
        groundShader->setMat4("viewingMatrix", viewingMatrix);
        groundShader->setMat4("modelingMatrix", modelingMatrix);
        // render ground
        glBindVertexArray(gVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // draw skybox as last
        skybox->shader->use();
        glm::mat4 view = glm::mat4(glm::mat3(viewingMatrix)); // remove translation from the view matrix
        skybox->shader->setMat4("view", view);
        skybox->shader->setMat4("projection", projectionMatrix);
        skybox->draw();
    }

    // bind back to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void display()
{

//    viewingMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
//    cout << "FPS: " << 1 / deltaTime << endl;

    // draw the scene to the framebuffer
    drawFrameBuffer();


    // reset color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
//    glViewport(0, 0, WIDTH, HEIGHT);



    //armadillo transformation
    armadillo->transform->SetModelMatrix();
    modelingMatrix = armadillo->transform->ModelMatrix;
    // Set the viewing matrix and the projection matrix
    viewingMatrix = camera.GetViewMatrix();
    projectionMatrix = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
    // Set the active program and the values of its uniform variables
    armadillo->shader->use();
    armadillo->shader->setMat4("projectionMatrix", projectionMatrix);
    armadillo->shader->setMat4("viewingMatrix", viewingMatrix);
    armadillo->shader->setMat4("modelingMatrix", modelingMatrix);
    armadillo->shader->setVec3("eyePos", camera.transform->Position);
    // Draw the scene
    armadillo->draw();

    //teapot transformation
    teapot->transform->SetModelMatrix();
    modelingMatrix = teapot->transform->ModelMatrix;
    // Set the active program and the values of its uniform variables
    teapot->shader->use();
    teapot->shader->setMat4("projectionMatrix", projectionMatrix);
    teapot->shader->setMat4("viewingMatrix", viewingMatrix);
    teapot->shader->setMat4("modelingMatrix", modelingMatrix);
    teapot->shader->setVec3("eyePos", camera.transform->Position);
    // Draw the scene
    teapot->draw();


    //cybertruck tranformation
    cybertruck->transform->SetModelMatrix();
    modelingMatrix = cybertruck->transform->ModelMatrix;
    // Camera transformation
    camera.transform->Position = cybertruck->transform->Position + cameraOffset;
    // Set the viewing matrix and the projection matrix
    viewingMatrix = camera.GetViewMatrix();
    projectionMatrix = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
    // Set the active program and the values of its uniform variables
    cybertruck->shader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeBuffer);
    cybertruck->shader->setMat4("projectionMatrix", projectionMatrix);
    cybertruck->shader->setMat4("viewingMatrix", viewingMatrix);
    cybertruck->shader->setMat4("modelingMatrix", modelingMatrix);
    cybertruck->shader->setVec3("eyePos", camera.transform->Position);
    cybertruck->shader->setFloat("factor", carFactor);
    // Draw the scene
    cybertruck->draw();

    // Set the active program and the values of its uniform variables
    cbWindows->shader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeBuffer);
    cbWindows->shader->setMat4("projectionMatrix", projectionMatrix);
    cbWindows->shader->setMat4("viewingMatrix", viewingMatrix);
    cbWindows->shader->setMat4("modelingMatrix", modelingMatrix);
    cbWindows->shader->setVec3("eyePos", camera.transform->Position);
    cybertruck->shader->setFloat("factor", windowFactor);

    // Draw the scene
    cbWindows->draw();

    // Set the active program and the values of its uniform variables
    cbTires->shader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeBuffer);
    cbTires->shader->setMat4("projectionMatrix", projectionMatrix);
    cbTires->shader->setMat4("viewingMatrix", viewingMatrix);
    cbTires->shader->setMat4("modelingMatrix", modelingMatrix);
    cbTires->shader->setVec3("eyePos", camera.transform->Position);
    // Draw the scene
    cbTires->draw();

    // textures
    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, groundTextureID);
    // texture transformation
    glm::mat4 matRy = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
    glm::mat4 matS = glm::scale(glm::mat4(1.0f), glm::vec3(100.0f, 0.0f, 100.0f));
    glm::mat4 matT = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.15f, 0.0f));
    modelingMatrix = matT * matS * matRy;
    // render container
    groundShader->use();
    groundShader->setMat4("projectionMatrix", projectionMatrix);
    groundShader->setMat4("viewingMatrix", viewingMatrix);
    groundShader->setMat4("modelingMatrix", modelingMatrix);
    // render ground
    glBindVertexArray(gVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // draw skybox as last
    skybox->shader->use();
    glm::mat4 view = glm::mat4(glm::mat3(viewingMatrix)); // remove translation from the view matrix
    skybox->shader->setMat4("view", view);
    skybox->shader->setMat4("projection", projectionMatrix);
    skybox->draw();
}

void reshape(GLFWwindow* window, int w, int h)
{
    w = w < 1 ? 1 : w;
    h = h < 1 ? 1 : h;

    gWidth = w;
    gHeight = h;

    glViewport(0, 0, w, h);

    // Use perspective projection

    float fovyRad = (float)(45.0 / 180.0) * M_PI;
    projectionMatrix = glm::perspective(fovyRad, w/(float) h, 1.0f, 100.0f);

    viewingMatrix = glm::mat4(1);

}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        cybertruck->transform->MoveTowards(-0.1f);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        cybertruck->transform->MoveTowards(0.1f);
    }
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
    {
        cybertruck->transform->Strafe(-0.1f);
    }
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
    {
        cybertruck->transform->Strafe(0.1f);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        cybertruck->transform->Rotate(0.f, 90.0f, 0.f);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        cybertruck->transform->Rotate(0.f, -90.0f, 0.f);
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        camera.SetYaw(180.f);
        cameraOffset = cybertruck->transform->Right * 5.0f + cybertruck->transform->Up * 2.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        camera.SetYaw(0.f);
        cameraOffset = cybertruck->transform->Right * -5.0f + cybertruck->transform->Up * 2.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        camera.SetYaw(90.f);
        cameraOffset = cybertruck->transform->Front * 7.0f + cybertruck->transform->Up * 2.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
    {
        camera.SetYaw(270.f);
        cameraOffset = cybertruck->transform->Front * -7.0f + cybertruck->transform->Up * 2.0f;
    }
    if(glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if(glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    camera.updateCameraVectors();

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void mainLoop(GLFWwindow* window)
{
    while (!glfwWindowShouldClose(window))
    {
        display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

int main(int argc, char** argv)   // Create Main Function For Bringing It All Together
{
    GLFWwindow* window;
    if (!glfwInit())
    {
        exit(-1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    window = glfwCreateWindow(WIDTH, HEIGHT, "CENG469-THE2", NULL, NULL);

    cout << "Welcome to Dynamic Environment Generator" << endl;
    cout << "=================================" << endl;
    cout << "------->This program is the second programming assignment "
            "\nof CENG469 Advanced Computer Graphics course." << endl;
    cout << "Press:" << endl;
    cout << "\t 'You can look around with mouse" << endl;
    cout << "\t 'W': move car forwards" << endl;
    cout << "\t 'S': move car backwards" << endl;
    cout << "\t 'A': rotate 90 degrees ccw" << endl;
    cout << "\t 'D': rotate 90 degrees cw" << endl;
    cout << "\t 'X': strafe left" << endl;
    cout << "\t 'C': strafe right" << endl;
    cout << "\t 'Q': move camera to the left side of the car" << endl;
    cout << "\t 'E': move camera to the right side of the car" << endl;
    cout << "\t 'R': move camera to the front side of the car" << endl;
    cout << "\t 'T': move camera to the back side of the car" << endl;
    cout << "\t 'K': unlock the cursor" << endl;
    cout << "\t 'L': lock the cursor" << endl;
    cout << "\t 'ESC': to quit" << endl;
    cout << endl << endl;
    cout << "\t ' Developed by Ali Komurcu '" << endl;

    if (!window)
    {
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Initialize GLEW to setup the OpenGL Function pointers
    if (GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    char rendererInfo[512] = { 0 };
    strcpy(rendererInfo, (const char*)glGetString(GL_RENDERER));
    strcat(rendererInfo, " - ");
    strcat(rendererInfo, (const char*)glGetString(GL_VERSION));
    glfwSetWindowTitle(window, rendererInfo);

    init();

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window, keyboard);
    glfwSetWindowSizeCallback(window, reshape);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    reshape(window, WIDTH, HEIGHT); // need to call this once ourselves
    mainLoop(window); // this does not return unless the window is closed

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
