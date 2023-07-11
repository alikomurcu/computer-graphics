#ifndef CAMERA_H
#define CAMERA_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;
// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const float YAW         = 90.0f;
const float PITCH       =  0.0f;
const float ROLL        =  0.0f;
const float SPEED       =  5.0f;
const float SENSITIVITY =  0.1f;
const float ZOOM        =  90.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // camera Attributes
    Transform* transform;
    glm::vec3 WorldUp;

    // euler Angles
    float Yaw;
    float Pitch;
    float Roll;
    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    glm::mat4 viewMatrix;

    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH, float roll = ROLL) : MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        transform = new Transform();
        transform->Position = position;
        transform->Front = glm::vec3(0.0f, 0.0f, -1.0f);
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }
    // constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch, float roll) : MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        transform->Position = glm::vec3(posX, posY, posZ);
        transform->Front = glm::vec3(0.0f, 0.0f, -1.0f);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        Roll = roll;
        updateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        glm::vec3 front;
        front.x = glm::sin(glm::radians(Yaw)) * glm::cos(glm::radians(Pitch));
        front.y = -glm::sin(glm::radians(Pitch));
        front.z = glm::cos(glm::radians(Yaw)) * glm::cos(glm::radians(Pitch));
        transform->Front = glm::normalize(front);
        transform->Front.z = -transform->Front.z;
        // also re-calculate the Right and Up vector

//        transform->Right = glm::normalize(glm::cross(transform->Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        glm::vec3 right;
        right.x = glm::cos(glm::radians(Yaw));
        right.y = 0;
        right.z = -glm::sin(glm::radians(Yaw));
        transform->Right = glm::normalize(right);

        transform->Up    = glm::normalize(glm::cross(transform->Front, transform->Right));

//        glm::quat rotation = glm::quat(glm::vec3(Pitch, Yaw, Roll));
//        glm::mat4 rotationMatrix = glm::mat4_cast(rotation);
//        transform->Front = glm::vec3(rotationMatrix[2]);
//        transform->Up = glm::vec3(rotationMatrix[1]);
//        transform->Right = glm::vec3(rotationMatrix[0]);

        glm::quat qPitch = glm::angleAxis(glm::radians(Pitch), glm::vec3(1, 0, 0));
        glm::quat qYaw = glm::angleAxis(glm::radians(Yaw), glm::vec3(0, 1, 0));
        glm::quat qRoll = glm::angleAxis(glm::radians(Roll),glm::vec3(0,0,1));

        //For an FPS camera we can omit roll
        glm::quat orientation = qRoll * qPitch * qYaw;
        orientation = glm::normalize(orientation);
        glm::mat4 rotate = glm::mat4_cast(orientation);
//        transform->Front = glm::vec3(rotate[2]);
//        transform->Up = glm::vec3(rotate[1]);
//        transform->Right = glm::vec3(rotate[0]);

        glm::mat4 translate = glm::mat4(1.0f);
        translate = glm::translate(translate, -transform->Position);

        viewMatrix = rotate * translate;
        return viewMatrix;
    }

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            transform->Position += transform->Front * velocity;
        if (direction == BACKWARD)
            transform->Position -= transform->Front * velocity;
        if (direction == LEFT)
            transform->Position -= transform->Right * velocity;
        if (direction == RIGHT)
            transform->Position += transform->Right * velocity;
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw   += xoffset;
        Pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
//        if (constrainPitch)
//        {
//            if (Pitch > 89.0f)
//                Pitch = 89.0f;
//            if (Pitch < -89.0f)
//                Pitch = -89.0f;
//        }

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }

    void SetYaw(float angle)
    {
        Yaw = angle;
    }

    void SetPitch(float angle)
    {
        Pitch = angle;
    }

    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
//        // calculate the new Front vector
//        glm::vec3 front;
//        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
//        front.y = sin(glm::radians(Pitch));
//        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
//        transform->Front = glm::normalize(front);
//        // also re-calculate the Right and Up vector
//        transform->Right = glm::normalize(glm::cross(transform->Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
//        transform->Up    = glm::normalize(glm::cross(transform->Right, transform->Front));

//        RotatePitch(glm::radians(Pitch));
//        RotateYaw(glm::radians(Yaw));
//        RotateRoll(glm::radians(Roll));

    }

    void RotatePitch(float rads) // rotate around cams local X axis
    {
        glm::quat qPitch = glm::angleAxis(rads, glm::vec3(1, 0, 0));

        transform->Rotation = glm::normalize(qPitch) * transform->Rotation;
        glm::mat4 rotate = glm::mat4_cast(transform->Rotation);

        glm::mat4 translate = glm::mat4(1.0f);
        translate = glm::translate(translate, -transform->Position);

        viewMatrix = rotate * translate;
    }
    void RotateYaw(float rads) // rotate around cams local X axis
    {
        glm::quat qYaw = glm::angleAxis(rads, glm::vec3(0, 1, 0));

        transform->Rotation = glm::normalize(qYaw) * transform->Rotation;
        glm::mat4 rotate = glm::mat4_cast(transform->Rotation);

        glm::mat4 translate = glm::mat4(1.0f);
        translate = glm::translate(translate, -transform->Position);

        viewMatrix = rotate * translate;
    }

    void RotateRoll(float rads) // rotate around cams local X axis
    {
        glm::quat qRoll = glm::angleAxis(rads, glm::vec3(0, 0, 1));

        transform->Rotation = glm::normalize(qRoll) * transform->Rotation;
        glm::mat4 rotate = glm::mat4_cast(transform->Rotation);

        glm::mat4 translate = glm::mat4(1.0f);
        translate = glm::translate(translate, -transform->Position);

        viewMatrix = rotate * translate;
    }
};
#endif
