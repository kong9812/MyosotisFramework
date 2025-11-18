// Copyright (c) 2025 kong9812
#pragma once
#include <vector>
#include "meshlet.h"

namespace MyosotisFW
{
	struct Mesh
	{
		std::vector<float> vertex;
		std::vector<Meshlet> meshlet;
		glm::vec3 min;
		glm::vec3 max;
	};
}
