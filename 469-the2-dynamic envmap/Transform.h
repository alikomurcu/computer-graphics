#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#ifndef THE2_TRANSFORM_H
#define THE2_TRANSFORM_H

// This is a class for handling transformations
class Transform {
private:
    glm::mat4 matT, matR, matS;
public:
    // Transform attributes
    glm::vec3 Position;
    glm::vec3 Scale;
    glm::vec3 Rotation;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::mat4 ModelMatrix;

    Transform() {
        Position = glm::vec3(-0.1f, -0.2f, -7.0f);
        Scale = glm::vec3(1.0f, 1.0f, 1.0f);
        Rotation = glm::vec3(0.0f, 0.0f, 0.0f);
        Up = glm::vec3(0.0f, 1.0f, 0.0f);
        Right = glm::vec3(1.0f, 0.0f, 0.0f);
        Front = glm::vec3(0.0f, 0.0f, -1.0f);
        ModelMatrix = glm::mat4(1.0f);
        matT = glm::translate(glm::mat4(1.0), Position);
        matR = glm::mat4(1.0);

    }

    void SetModelMatrix()
    {
        SetMatrices();
        ModelMatrix = matT * matR * matS;
    }

    void MoveTowards(float move)
    {
        Position += Front * move;
    }

    void Strafe(float move)
    {
        Position += Right * move;
    }

    void clampRotation()
    {
        if (Rotation.x > 360.0f)
            Rotation.x -= 360.0f;
        if (Rotation.y > 360.0f)
            Rotation.y -= 360.0f;
        if (Rotation.z > 360.0f)
            Rotation.z -= 360.0f;
        if (Rotation.x < 0.0f)
            Rotation.x += 360.0f;
        if (Rotation.y < 0.0f)
            Rotation.y += 360.0f;
        if (Rotation.z < 0.0f)
            Rotation.z += 360.0f;
    }

    void Rotate(float x, float y, float z)
    {
        std::cout << "Before Rotate: " << Rotation.x << " " << Rotation.y << " " << Rotation.z << std::endl;
        std::cout << "Before Front: " << Front.x << " " << Front.y << " " << Front.z << std::endl;
        std::cout << "Before Right: " << Right.x << " " << Right.y << " " << Right.z << std::endl;
        Rotation += glm::vec3(x, y, z);
        clampRotation();
        std::cout << "After Rotate: " << Rotation.x << " " << Rotation.y << " " << Rotation.z << std::endl;
//        glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), glm::radians(y), glm::vec3(0.0f, 1.0f, 0.0f));
//        Front = glm::normalize(glm::vec3(rotate * glm::vec4(Front, 1.0f)));
        int yAngle = (int) (Rotation.y + 360) % 360;
        std::cout << "Rotation.y: " << Rotation.y << std::endl;
        std::cout << "yAngle: " << yAngle << std::endl;
        if (yAngle == 0)
            Front = glm::vec3(0.0f, 0.0f, -1.0f);
        else if (yAngle == 90)
            Front = glm::vec3(-1.0f, 0.0f, 0.0f);
        else if (yAngle == 180)
            Front = glm::vec3(0.0f, 0.0f, 1.0f);
        else if (yAngle == 270)
            Front = glm::vec3(1.0f, 0.0f, 0.0f);

        std::cout << "After Front: " << Front.x << " " << Front.y << " " << Front.z << std::endl;
        Right = glm::normalize(glm::cross(Front, Up));
        std::cout << "After Right: " << Right.x << " " << Right.y << " " << Right.z << std::endl;
    }

    void ScaleUp(float x, float y, float z)
    {
        Scale *= glm::vec3(x, y, z);
    }

private:
    void SetMatrices()
    {
        // Translation
        matT = glm::translate(glm::mat4(1.0), Position);
        // Rotations
        matR = glm::rotate<float>(glm::mat4(1.0), ((float)Rotation.x / 180.0) * M_PI, glm::vec3(1.0f, 0.0f, 0.0f));
        matR = glm::rotate<float>(matR, ((float)Rotation.y / 180.0) * M_PI, glm::vec3(0.0f, 1.0f, 0.0f));
        matR = glm::rotate<float>(matR, ((float)Rotation.z / 180.0) * M_PI, glm::vec3(0.0f, 0.0f, 1.0f));

        // Scale
        matS = glm::scale(glm::mat4(1.0), Scale);
    }
};
#endif //THE2_TRANSFORM_H
