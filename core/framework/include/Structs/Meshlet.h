// Copyright (c) 2025 kong9812
#pragma once
#include <vector>
#include "iglm.h"

namespace MyosotisFW
{
	struct Meshlet
	{
		std::vector<uint32_t> uniqueIndex;
		std::vector<uint32_t> primitives;
		glm::vec3 min;
		glm::vec3 max;
	};
}
