// Copyright (c) 2025 kong9812
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

#include "classPointer.h"
#include "vkStruct.h"

namespace MyosotisFW::System::Render::Camera
{
    class CameraBase
    {
    public:
        CameraBase();
        virtual ~CameraBase() = default;

        virtual glm::mat4 GetViewMatrix() const;
        virtual glm::mat4 GetProjectionMatrix() const;
        float GetDistance(glm::vec3 pos) const;

        virtual void Update(Utility::Vulkan::Struct::UpdateData updateData) = 0;
    protected:
        // カメラ位置
        glm::vec3 m_cameraPos;
        // カメラ視点
        glm::vec3 m_cameraLookAt;
        // カメラ　前ベクトル
        glm::vec3 m_cameraFront;
        // カメラ　上ベクトル
        glm::vec3 m_cameraUp;
        // カメラ　右ベクトル
        glm::vec3 m_cameraRight;
        // カメラ FOV
        float m_cameraFov;
        // カメラ Far
        float m_cameraFar;
        // カメラ Near
        float m_cameraNear;
        // アスペクト比
        float m_aspectRadio;
    };
}