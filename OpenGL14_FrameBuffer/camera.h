#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
//枚举类，定义几个可能的相机移动选项。用作抽象，以避免使用特定于窗口系统的输入方法
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;//移动速度
const float SENSITIVITY = 0.1f;//灵敏度
const float ZOOM = 45.0f;//视野角度


class Camera {//摄像机类
public:
	//摄像机属性
    glm::vec3 Position;//位置
    glm::vec3 Front;//原点位置
    glm::vec3 Up;//向上向量
    glm::vec3 Right;//向右向量
    glm::vec3 WorldUp;//空间向上向量

    //欧拉角
    float Yaw;//yaw偏航角
    float Pitch;//俯仰角

    //相机
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    //构造函数函数
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM){
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();//更新相机属性
    }
    //标量参数的构造函数
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();//更新相机属性
    }

    //获取视野矩阵：lookAt矩阵
    glm::mat4 GetViewMatrix() {
        return glm::lookAt(Position, Position + Front, Up);
    }

    //根据键盘输入，更新相机位置，移动相机
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
        // make sure the user stays at the ground level
        //Position.y = 0.0f; //使不能在y轴转动
    }

    //根据鼠标输入，更新视线角度，偏航角和俯仰角
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;
        Yaw += xoffset;
        Pitch += yoffset;
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }
        updateCameraVectors();//更新相机属性
    }

    //根据鼠标滑轮，更新视野角度Fov
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }


private:
    //更新相机属性
    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);//向前向量，标准化
        Right = glm::normalize(glm::cross(front, WorldUp));//向右向量
        Up = glm::normalize(glm::cross(Right, Front));//向上向量

    }
};
#endif