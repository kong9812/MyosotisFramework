// Copyright (c) 2025 kong9812
#pragma once
#include <string>
#include <unordered_map>
#include "istduuid.h"

namespace MyosotisFW
{
	// todo. bit fragに変える
	// CustomMesh/GeomMesh が StaticMeshに含むかどうかを判定するときに、
	// Cameraなら　FPSカメラがCameraに含むかどうかを判定するときに使う
	typedef enum
	{
		Undefined = -1,
		FPSCamera,
		PrimitiveGeometryMesh,
		CustomMesh,
		Terrain,

		Max,
		Begin = FPSCamera,
	}ComponentType;

	constexpr const char* ComponentTypeName[static_cast<uint32_t>(ComponentType::Max)] =
	{
		"FPSCamera",
		"PrimitiveGeometryMesh",
		"CustomMesh",
		"Terrain",
	};

	const std::unordered_map<ComponentType, std::optional<uuids::uuid>> g_objectTypeUUID = {
		{ ComponentType::Undefined,					uuids::uuid::from_string("") },
		{ ComponentType::FPSCamera,					uuids::uuid::from_string("e1bbd200-e5ee-4587-8ef8-39b3cc343984") },
		{ ComponentType::PrimitiveGeometryMesh,		uuids::uuid::from_string("7f3512b5-4cc9-4caa-a8a9-76d05723e415") },
		{ ComponentType::CustomMesh,				uuids::uuid::from_string("6eeb286b-2352-4a66-92f6-6c8b75736337") },
		{ ComponentType::Terrain,					uuids::uuid::from_string("8017334a-494e-4b3b-92f6-we1e4536451w") },
		{ ComponentType::Max,						uuids::uuid::from_string("") },
	};

	inline ComponentType findComponentTypeFromTypeID(const uuids::uuid& typeID)
	{
		auto it = std::find_if(g_objectTypeUUID.begin(), g_objectTypeUUID.end(), [&](const std::pair<ComponentType, std::optional<uuids::uuid>>& element)
			{
				return element.second == typeID;
			});
		return it == g_objectTypeUUID.end() ? ComponentType::Undefined : it->first;
	}
}