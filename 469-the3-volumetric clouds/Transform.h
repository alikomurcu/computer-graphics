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
    glm::quat Rotation;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::mat4 ModelMatrix;

    Transform() {
        Position = glm::vec3(-0.1f, -0.2f, -7.0f);
        Scale = glm::vec3(1.0f, 1.0f, 1.0f);
        Rotation = glm::quat (glm::vec3(0.0f, 0.0f, 0.0f));
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

    void Rotate(float x, float y, float z)
    {
        Rotation = glm::quat(glm::vec3(x, y, z));

        matR = glm::toMat4(Rotation);
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

        // Scale
        matS = glm::scale(glm::mat4(1.0), Scale);
    }
};
#endif //THE2_TRANSFORM_H
