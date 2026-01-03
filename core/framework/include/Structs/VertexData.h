// Copyright (c) 2025 kong9812
#pragma once
#include "iglm.h"

namespace MyosotisFW
{
	struct VertexData
	{
		glm::vec3 position = glm::vec3(0.0f);
		glm::vec3 normal = glm::vec3(0.0f);
		glm::vec2 uv0 = glm::vec2(0.0f);
		glm::vec2 uv1 = glm::vec2(0.0f);
		glm::vec4 color = glm::vec4(0.0f);
	};
}