// Copyright (c) 2025 kong9812
#pragma once
#include <vector>
#include "MeshletInfo.h"

namespace MyosotisFW
{
	struct Meshlet
	{
		// Info
		MeshletInfo meshletInfo{};

		std::vector<uint32_t> uniqueIndex{};
		std::vector<uint32_t> primitives{};
	};
}
