#include "main.h"

unsigned int framebuffer;
unsigned int cubeBuffer;
unsigned int rbo;

int gWidth, gHeight;

glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix;
glm::mat4 modelingMatrix;
glm::mat4 Rot;

// models details
Object *cybertruck, *ground, *plane, *quad;
Skybox* skybox;

// shaders
Shader *groundShader;
unsigned int gVAO;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastTime = 0.0f;
float angles = 0.0f;

// mouse
float xpos, ypos;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)WIDTH / 2.0;
float lastY = (float)HEIGHT / 2.0;
bool firstMouse = true;
glm::vec3 cameraOffset(0.0f, 2.0f, -7.0f);
std::vector<Camera> cameraList;
float movement = 0.0f, ix = 0.0f, iy = 0.0f, iz = 0.0f;

// Ground texture
GLuint groundTextureID, quadTexture;

// cloud related
bool drawCloud = true;


void initVBO()
{
    skybox->loadCubemap();
    // load models
    quad->InitObject();
}

void handle_cloud()
{
    int width = WIDTH/2, height = HEIGHT/2, depth = 4;

    glGenTextures(1, &quadTexture);
    glBindTexture(GL_TEXTURE_3D, quadTexture);


    std::vector<glm::vec4> textureData(width * height * depth);
//    for (int z = 0; z < depth; ++z) {
//        for (int y = 0; y < height; ++y) {
//            for (int x = 0; x < width; ++x) {
//                // Generate random RGBA values for each voxel
//                glm::vec4 voxelColor = glm::vec4(
//                        static_cast<float>(rand()) / RAND_MAX,  // Red component
//                        static_cast<float>(rand()) / RAND_MAX,  // Green component
//                        static_cast<float>(rand()) / RAND_MAX,  // Blue component
//                        static_cast<float>(rand()) / RAND_MAX   // Alpha component
//                );
//                int index = x + y * width + z * height * width;
//                textureData[index] = voxelColor;
//            }
//        }
//    }
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, width, height, depth, 0, GL_RED,
                 GL_UNSIGNED_BYTE, textureData.data());

    // set the sampler
    GLuint mySampler;
    glGenSamplers(1, &mySampler);
    glBindSampler(0, mySampler);

    GLfloat param[] = {0.0f, 0.0f, 0.0f, 0.0f};
    glSamplerParameteri(mySampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(mySampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(mySampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glSamplerParameteri(mySampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glSamplerParameteri(mySampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    glSamplerParameterfv(mySampler, GL_TEXTURE_BORDER_COLOR, param);

    glBindTexture(GL_TEXTURE_3D, 0);
    glBindSampler(0, 0);
}

void handle_texture()
{
    groundTextureID = loadTexture("textures/ground_texture_sand.jpg");
    groundShader = new Shader("shaders/groundtexvs.glsl", "shaders/groundtexfs.glsl");
    float vertices[] = {
            // positions                        // colors                        // texture coords
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

void init()
{
    glEnable(GL_DEPTH_TEST);

    ground = new Object("obj/ground.obj");
    ground->initShader("shaders/groundtexvs.glsl", "shaders/groundtexfs.glsl");

    skybox = new Skybox();
    skybox->initShader("shaders/skyvs.glsl", "shaders/skyfs.glsl");
    skybox->shader->use();
    skybox->shader->setInt("skybox", 0);

    quad = new Object("obj/quad.obj");
    quad->initShader("shaders/quadvs.glsl", "shaders/quadfs.glsl");

    initVBO();

    // cloud texture
    handle_cloud();
    // texture
    handle_texture();
}

void display()
{
//    viewingMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
//    float currentFrame = static_cast<float>(glfwGetTime());
//    deltaTime = currentFrame - lastFrame;
//    lastFrame = currentFrame;
//    cout << "FPS: " << 1 / deltaTime << endl;

    // reset color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // Set the viewing matrix and the projection matrix
    viewingMatrix = camera.GetViewMatrix();
    projectionMatrix = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);

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

    if(drawCloud) {
        // draw cloud
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, groundTextureID);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_ALWAYS); // test always passes, enable writes
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // TODO:

        // make quad in front of camera

        // check 3 seconds passed

        //    if (glfwGetTime() - lastTime > 1.0f) {
        //        // update lastTime
        //        lastTime = glfwGetTime();
        //        glm::quat rotation = glm::angleAxis(glm::radians(angles), glm::vec3(0.0, 1.0, 0.0));
        //        Rot = glm::mat4_cast(rotation);
        //        angles += 10.0f;
        //    }

        //    glm::mat4 Rot = glm::rotate(glm::mat4(1.0f), glm::radians(camera.Pitch), glm::vec3(1.0, 0.0, 0.0));
        //    glm::rotate(Rot, glm::radians(camera.Yaw), glm::vec3(0.0, 1.0, 0.0));
        viewingMatrix = glm::lookAt(glm::vec3(0, 0, 1), glm::vec3(0, 0, -1), glm::vec3(0.0f, 1.0f, 0.0f));
        //    modelingMatrix = Trans * Rot2 * Rot;

        modelingMatrix = glm::mat4(1.0f);


        quad->shader->use();
        quad->shader->setMat4("projectionMatrix", projectionMatrix);
        quad->shader->setMat4("viewingMatrix", viewingMatrix);
        quad->shader->setMat4("modelingMatrix", modelingMatrix);
        // get current time
        float currentFrame = static_cast<float>(glfwGetTime());

        quad->shader->setFloat("iTime", currentFrame);
        quad->shader->setFloat("ix", ix);
        quad->shader->setFloat("iy", iy);
        quad->shader->setFloat("iz", iz);
        quad->shader->setVec2("resolution", glm::vec2(WIDTH, HEIGHT));
        quad->shader->setVec2("iMouse", glm::vec2(xpos, ypos));
        quad->draw();

        glDepthFunc(GL_LESS); // set depth function back to default
        glDisable(GL_BLEND);
    }
    cout << "Pitch " << camera.Pitch * 10 << " Yaw " << camera.Yaw << " Roll " << camera.Roll << " Front "
         << (int)(camera.transform->Front.x * 10) << " " << (int)(camera.transform->Front.y * 10) << " " << (int)(camera.transform->Front.z * 10)
         << endl;
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
    projectionMatrix = glm::perspective(fovyRad, (float)w/(float) h, 1.0f, 100.0f);

    viewingMatrix = glm::mat4(1);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    xpos = static_cast<float>(xposIn);
    ypos = static_cast<float>(yposIn);
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
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)       // TODO: speedup logic
    {
        camera.transform->Position += camera.transform->Front * 0.1f;
        iz += 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera.transform->Position -= camera.transform->Front * 0.1f;
        iz -= 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        iz -= 1.0f;
        ix -= 1.0f;
        iy += 1.0f;
        camera.Roll -= 1.0f;
        if (((int)camera.Roll % 360) == 0)
            iy = 0.f;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        iz += 1.0f;
        ix += 1.0f;
        iy += 1.0f;
        camera.Roll += 1.0f;
        if (((int)camera.Roll % 360) == 0)
            iy = 0.f;
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        camera.Yaw -= 1.f;
        ix -= 1.f;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        camera.Yaw += 1.f;
        ix += 1.f;
    }
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
    {
        camera.Pitch -= 1.f;
        if (((int)camera.Pitch % 360) == 0)
            iy = 0.f;
        iy -= 1.f;
    }
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
    {
        camera.Pitch += 1.f;
        if (((int)camera.Pitch % 360) == 0)
            iy = 0.f;
        iy += 1.f;
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
    {
        drawCloud = !drawCloud;
    }
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
    {
        ix += 1.f;
    }
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
    {
        ix -= 1.f;
    }
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
    {
        iz += 1.f;
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
    {
        iz -= 1.f;
    }
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
    {
        iy += 1.f;
    }
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
    {
        iy -= 1.f;
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

    cout << "Welcome to " << endl;
    cout << "=================================" << endl;
    cout << "------->This program is the third programming assignment "
            "\nof CENG469 Advanced Computer Graphics course." << endl;

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
//    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
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
