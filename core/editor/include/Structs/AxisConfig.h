// Copyright (c) 2025 kong9812
#pragma once
#include "iglm.h"

namespace MyosotisFW
{
	struct AxisConfig
	{
		glm::vec3 rotation = glm::vec3(0.0f);
		glm::vec3 direction = glm::vec3(0.0f);

		glm::vec4 normalColor = glm::vec4(0.0f);
		glm::vec4 hoverColor = glm::vec4(0.0f);
		glm::vec4 pressedColor = glm::vec4(0.0f);
	};
}