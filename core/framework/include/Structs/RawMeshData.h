// Copyright (c) 2025 kong9812
#pragma once
#include <vector>
#include "VertexData.h"

namespace MyosotisFW
{
	struct RawMeshData
	{
		std::vector<VertexData> vertex;
		std::vector<uint32_t> index;
	};
}
