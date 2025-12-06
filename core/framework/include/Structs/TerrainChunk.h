// Copyright (c) 2025 kong9812
#pragma once
#include "iglm.h"

namespace MyosotisFW
{
	struct TerrainChunk
	{
		glm::ivec2 chunkCoord;
		glm::ivec2 offset;
		glm::ivec2 size;
		uint32_t _p1;
		uint32_t _p2;
	};
}
