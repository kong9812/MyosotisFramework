// Copyright (c) 2025 kong9812
#pragma once
#include "iglm.h"

namespace MyosotisFW
{
	struct BasicMaterialInfo
	{
		// ======================================================================================================
		// bit 0～7
		// 0: HasBaseColorTexture
		// 1: HasNormalTexture
		// 2: HasORMTexture
		// 3: HasEmissiveTexture
		// 4: UseVertexColor (true: vertexColor false: (hasBaseColorTexture ? hasBaseColorTexture : baseColor))
		// 5: [Test] Reflection
		// 6～7: Undefine
		// ======================================================================================================
		// bit 8～31
		// Undefine
		// ======================================================================================================
		uint32_t bitFlags = 0;

		uint32_t baseColorTexture = 0;
		uint32_t normalTexture = 0;
		uint32_t p3 = 0;

		glm::vec4 baseColor = glm::vec4(0.0f);
	};
}
