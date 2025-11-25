// Copyright (c) 2025 kong9812
#pragma once
#include "ClassPointer.h"
#include "Transform.h"

namespace MyosotisFW
{
	struct ObjectInfo
	{
		Transform transform;
		glm::mat4 model;
		uint32_t objectID;
		uint32_t meshID;
		uint32_t materialID;
		uint32_t vbDispatchInfoStartIndex;
	};
	TYPEDEF_SHARED_PTR(ObjectInfo);
}
