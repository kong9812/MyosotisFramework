// Copyright (c) 2025 kong9812
#include "fpsCamera.h"

namespace MyosotisFW::System::Render::Camera
{
	FPSCamera::FPSCamera(glm::vec3 position, glm::vec3 worldUp)
	{
		m_position = position;
		m_yaw = -90.0f;
		m_pitch = 0.0f;
		m_worldUp = worldUp;

		updateCameraVectors();
	}
}