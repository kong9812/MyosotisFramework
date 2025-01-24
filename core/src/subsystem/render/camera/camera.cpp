// Copyright (c) 2025 kong9812
#include "camera.h"
#include "appInfo.h"

namespace MyosotisFW::System::Render::Camera
{
    CameraBase::CameraBase() :ObjectBase(ObjectType::Camera)
	{
        // カメラ位置
        m_cameraPos = AppInfo::g_cameraPos;
        // カメラ視点
        m_cameraLookAt = AppInfo::g_cameraLookAt;
        // カメラ　前ベクトル
        m_cameraFront = AppInfo::g_cameraFront;
        // カメラ　上ベクトル
        m_cameraUp = AppInfo::g_cameraUp;
        // カメラ　右ベクトル
        m_cameraRight = AppInfo::g_cameraRight;
        // カメラ FOV
        m_cameraFov = AppInfo::g_cameraFov;
        // カメラ Far
        m_cameraFar = AppInfo::g_cameraFar;
        // カメラ Near
        m_cameraNear = AppInfo::g_cameraNear;

        // アスペクト比
        m_aspectRadio = static_cast<float>(AppInfo::g_windowWidth) / static_cast<float>(AppInfo::g_windowHeight);
	}

    glm::mat4 CameraBase::GetViewMatrix() const
    {
        return glm::lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);
    }

    glm::mat4 CameraBase::GetProjectionMatrix() const
    {
        return glm::perspective(glm::radians(m_cameraFov), m_aspectRadio, m_cameraNear, m_cameraFar);
    }

    float CameraBase::GetDistance(glm::vec3 pos) const
    {
        return glm::distance(m_cameraPos, pos);
    }
}
