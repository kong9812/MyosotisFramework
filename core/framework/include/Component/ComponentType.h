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
		Skybox,
		InteriorObjectMesh,
		Max
	}ComponentType;

	// todo. bitfrag対応するまでの一時対応 (対応したい消す)
	inline bool IsStaticMesh(const ComponentType& type)
	{
		return (type == ComponentType::PrimitiveGeometryMesh) || (type == ComponentType::CustomMesh);
	}

	const std::unordered_map<ComponentType, std::optional<uuids::uuid>> g_objectTypeUUID = {
		{ ComponentType::Undefined,				uuids::uuid::from_string("") },
		{ ComponentType::FPSCamera,				uuids::uuid::from_string("ecbbd200-e5ee-4587-8ef8-39bfcc343984") },
		{ ComponentType::PrimitiveGeometryMesh,	uuids::uuid::from_string("7f3512b5-4cc9-4caa-a8a9-76d05723e415") },
		{ ComponentType::CustomMesh,				uuids::uuid::from_string("6eeb286b-2352-4a66-92f6-6c8b75736337") },
		{ ComponentType::Skybox,					uuids::uuid::from_string("80173349-494e-42fc-9142-1f9a4f550d3c") },
		{ ComponentType::InteriorObjectMesh,		uuids::uuid::from_string("f3b3b3b4-3b3b-4b3b-8b3b-3b3b3b3b3b3b") },
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