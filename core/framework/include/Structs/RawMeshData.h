// Copyright (c) 2025 kong9812
#pragma once
#include "iglm.h"

namespace MyosotisFW
{
	struct RawMeshData
	{
		std::vector<glm::vec3> position;
		std::vector<glm::vec3> normal;
		std::vector<glm::vec2> uv;
		std::vector<glm::vec4> color;

		std::vector<float> vertex;
		std::vector<uint32_t> index;
	};
}
