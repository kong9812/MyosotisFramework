// Copyright (c) 2025 kong9812
#pragma once
#include <vector>
#include "Meshlet.h"
#include "MeshInfo.h"
#include "VertexData.h"

namespace MyosotisFW
{
	struct Mesh
	{
		// Info
		MeshInfo meshInfo;

		std::vector<VertexData> vertex;
		std::vector<Meshlet> meshlet;
	};
}
