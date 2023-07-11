#ifndef THE2_DYNAMICSKYBOX_H
#define THE2_DYNAMICSKYBOX_H

class DynamicSkybox: public Skybox {
public:
    unsigned int frameBuffer;
    unsigned int rbo;

    DynamicSkybox(): Skybox()
    {

    }
    void initFrameBuffer()
    {
        glGenTextures(1, &cubemapTexture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        int width, height;
        // set textures
        for (unsigned int i = 0; i < faces.size(); i++)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
        }
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


        // generate and bind framebuffer
        glGenFramebuffers(1, &frameBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

        // create rbo for depth and stencil attachment
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);
//        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WIDTH, HEIGHT);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);


        // attach rbo to framebuffer
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
//        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, frameBuffer);

        // attach cubemap texture to framebuffer
//        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, cubemapTexture, 0);
        for (int i = 0; i < 6; ++i)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemapTexture, 0);
        }
        // create the cubemap
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);   // unbind framebuffer
    }

    void drawFace(unsigned int face, Camera* camera, glm::vec3 carPos)
    {
        // set shader matrices
        glm::vec3 camPos = camera->transform->Position;
        glm::mat4 viewingMatrix = glm::mat4(glm::mat3(camera->GetViewMatrix()));

        // translate camera to car position
        camera->transform->Position = carPos;
        if(face == GL_TEXTURE_CUBE_MAP_POSITIVE_X)
            viewingMatrix = glm::lookAt(carPos, carPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.f, 1.f, 0.f));
        else if(face == GL_TEXTURE_CUBE_MAP_NEGATIVE_X)
            viewingMatrix = glm::lookAt(carPos, carPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.f, 1.f, 0.f));
        else if(face == GL_TEXTURE_CUBE_MAP_POSITIVE_Y)
            viewingMatrix = glm::lookAt(carPos, carPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.f, 0.f, 0.f));
        else if(face == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y)
            viewingMatrix = glm::lookAt(carPos, carPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(1.f, 0.f, 0.f));
        else if(face == GL_TEXTURE_CUBE_MAP_POSITIVE_Z)
            viewingMatrix = glm::lookAt(carPos, carPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.f, 1.f, 0.f));
        else if(face == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
            viewingMatrix = glm::lookAt(carPos, carPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.f, 1.f, 0.f));
        shader->setMat4("view", viewingMatrix);
        glm::mat4 projectionMatrix = glm::perspective(glm::radians(90.0f), 1.f, 1.f, 1000.0f);
        shader->setMat4("projection", projectionMatrix);

        // draw the scene
    }

    void drawDynamicCubemap(Camera* camera, glm::vec3 carPos)
    {
        for (unsigned int i = 0; i < faces.size(); i++)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemapTexture, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            drawFace(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, camera, carPos);
        }
        shader->use();
        // draw skybox as last
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

    }
};


#endif //THE2_DYNAMICSKYBOX_H
