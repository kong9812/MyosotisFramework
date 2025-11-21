// Copyright (c) 2025 kong9812
#pragma once
#include "ClassPointer.h"
#include "Transform.h"

namespace MyosotisFW
{
	struct ObjectInfo
	{
		Transform transform;
		uint32_t meshID;
		uint32_t materialID;
	};
	TYPEDEF_SHARED_PTR(ObjectInfo);
}
