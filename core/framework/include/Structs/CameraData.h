// Copyright (c) 2025 kong9812
#pragma once
#include "iglm.h"

namespace MyosotisFW
{
	struct CameraData
	{
		glm::vec4 frustumPlanes[6];
		glm::mat4 view;
		glm::mat4 projection;
		glm::mat4 invView;
		glm::mat4 invProjection;
		glm::vec4 position;
	};
}
