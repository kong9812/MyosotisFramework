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
	}ObjectType;

	// todo. bitfrag対応するまでの一時対応 (対応したい消す)
	inline bool IsStaticMesh(const ObjectType& type)
	{
		return (type == ObjectType::PrimitiveGeometryMesh) || (type == ObjectType::CustomMesh);
	}

	const std::unordered_map<ObjectType, std::optional<uuids::uuid>> g_objectTypeUUID = {
		{ ObjectType::Undefined,				uuids::uuid::from_string("") },
		{ ObjectType::FPSCamera,				uuids::uuid::from_string("ecbbd200-e5ee-4587-8ef8-39bfcc343984") },
		{ ObjectType::PrimitiveGeometryMesh,	uuids::uuid::from_string("7f3512b5-4cc9-4caa-a8a9-76d05723e415") },
		{ ObjectType::CustomMesh,				uuids::uuid::from_string("6eeb286b-2352-4a66-92f6-6c8b75736337") },
		{ ObjectType::Skybox,					uuids::uuid::from_string("80173349-494e-42fc-9142-1f9a4f550d3c") },
		{ ObjectType::InteriorObjectMesh,		uuids::uuid::from_string("f3b3b3b4-3b3b-4b3b-8b3b-3b3b3b3b3b3b") },
		{ ObjectType::Max,						uuids::uuid::from_string("") },
	};

	inline ObjectType findObjectTypeFromTypeID(const uuids::uuid& typeID)
	{
		auto it = std::find_if(g_objectTypeUUID.begin(), g_objectTypeUUID.end(), [&](const std::pair<ObjectType, std::optional<uuids::uuid>>& element)
			{
				return element.second == typeID;
			});
		return it == g_objectTypeUUID.end() ? ObjectType::Undefined : it->first;
	}
}