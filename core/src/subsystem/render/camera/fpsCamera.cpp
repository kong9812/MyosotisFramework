#include "fpsCamera.h"
// Copyright (c) 2025 kong9812

namespace MyosotisFW::System::Render::Camera
{
	glm::mat4 FPSCamera::GetViewMatrix() const
	{
		return glm::lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);
	}

	glm::mat4 FPSCamera::GetProjectionMatrix() const
	{
		return glm::perspective(glm::radians(m_cameraFov), m_aspectRadio, m_cameraNear, m_cameraFar);
	}
}