// Copyright (c) 2025 kong9812
#pragma once
#include "ClassPointer.h"
#include "Transform.h"

namespace MyosotisFW
{
	struct ObjectInfo
	{
		Transform transform{};
		glm::mat4 model = glm::mat4(0.0f);
		uint32_t objectID = 0;
		uint32_t meshID = 0;					// 使ってない
		uint32_t materialID = 0;
		uint32_t p1 = 0;
	};
	TYPEDEF_SHARED_PTR(ObjectInfo);
}
