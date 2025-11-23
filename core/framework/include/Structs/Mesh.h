// Copyright (c) 2025 kong9812
#pragma once
#include <vector>
#include "Meshlet.h"
#include "MeshInfo.h"

namespace MyosotisFW
{
	struct Mesh
	{
		// Info
		MeshInfo meshInfo;

		std::vector<float> vertex;
		std::vector<Meshlet> meshlet;
	};
}
