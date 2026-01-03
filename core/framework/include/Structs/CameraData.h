// Copyright (c) 2025 kong9812
#pragma once
#include "iglm.h"

namespace MyosotisFW
{
	struct CameraData
	{
		glm::vec4 frustumPlanes[6]{};
		glm::mat4 view = glm::mat4(0.0f);
		glm::mat4 projection = glm::mat4(0.0f);
		glm::mat4 invView = glm::mat4(0.0f);
		glm::mat4 invProjection = glm::mat4(0.0f);
		glm::vec4 position = glm::vec4(0.0f);
	};
}
