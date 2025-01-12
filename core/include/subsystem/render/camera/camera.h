// Copyright (c) 2025 kong9812
// Camera interface class
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

#include "classPointer.h"

namespace MyosotisFW::System::Render::Camera
{
    class CameraBase
    {
    public:
        CameraBase();
        virtual ~CameraBase() = default;

        virtual glm::mat4 GetViewMatrix() const;
        virtual glm::mat4 GetProjectionMatrix() const;

        virtual void ProcessMouseMovement(float xOffset, float yOffset) = 0;
        virtual void ProcessMouseZoom(float offset) = 0;
        virtual void ProcessKeyboardInput(const std::string& direction, float deltaTime) = 0;

    protected:
        // �J�����ʒu
        glm::vec3 m_cameraPos;
        // �J�������_
        glm::vec3 m_cameraLookAt;
        // �J�����@�O�x�N�g��
        glm::vec3 m_cameraFront;
        // �J�����@��x�N�g��
        glm::vec3 m_cameraUp;
        // �J�����@�E�x�N�g��
        glm::vec3 m_cameraRight;
        // �J���� FOV
        float m_cameraFov;
        // �J���� Far
        float m_cameraFar;
        // �J���� Near
        float m_cameraNear;
        // �A�X�y�N�g��
        float m_aspectRadio;
    };
}