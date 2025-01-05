// Copyright (c) 2025 kong9812
#include "camera.h"
#include "appInfo.h"

namespace MyosotisFW::System::Render::Camera
{
    CameraBase::CameraBase()
	{
        // �J�����ʒu
        m_cameraPos = AppInfo::g_cameraPos;
        // �J�������_
        m_cameraLookAt = AppInfo::g_cameraLookAt;
        // �J�����@�O�x�N�g��
        m_cameraFront = AppInfo::g_cameraFront;
        // �J�����@��x�N�g��
        m_cameraUp = AppInfo::g_cameraUp;
        // �J�����@�E�x�N�g��
        m_cameraRight = AppInfo::g_cameraRight;
        // �J���� FOV
        m_cameraFov = AppInfo::g_cameraFov;
        // �J���� Far
        m_cameraFar = AppInfo::g_cameraFar;
        // �J���� Near
        m_cameraNear = AppInfo::g_cameraNear;

        // �A�X�y�N�g��
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
}
