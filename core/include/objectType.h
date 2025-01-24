// Copyright (c) 2025 kong9812
#pragma once
#include <string>
#include <unordered_map>
#include "istduuid.h"

namespace MyosotisFW
{
	typedef enum
	{
		Undefined = -1,
		Camera,
		StaticMesh,
		Max
	}ObjectType;

	const std::unordered_map<ObjectType, std::optional<uuids::uuid>> g_objectTypeUUID = {
		{ ObjectType::Undefined,			uuids::uuid::from_string("") },
		{ ObjectType::Camera,				uuids::uuid::from_string("ecbbd200-e5ee-4587-8ef8-39bfcc343984") },
		{ ObjectType::StaticMesh,			uuids::uuid::from_string("7f3512b5-4cc9-4caa-a8a9-76d05723e415") },
		{ ObjectType::Max,					uuids::uuid::from_string("") },
	};
}