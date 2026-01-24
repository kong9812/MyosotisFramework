// Copyright (c) 2025 kong9812
#pragma once
#include "iglm.h"

namespace MyosotisFW
{
	struct AxisDrawCommand
	{
		glm::mat4 model = glm::mat4(0.0f);
		glm::vec4 color = glm::vec4(0.0f);
		float distance = 0.0f;
	};
}