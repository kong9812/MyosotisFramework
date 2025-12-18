// Copyright (c) 2025 kong9812
#pragma once
#include "MObjectRegistry.h"
#include "MObject.h"

namespace MyosotisFW
{
	MObject_ptr MObjectRegistry::CreateNewObject()
	{
		uint32_t objectIndex = static_cast<uint32_t>(m_object.size());
		MObject_ptr newObject = m_object.emplace_back(CreateMObjectPointer());
		ObjectInfo_ptr objectInfo = std::make_shared<ObjectInfo>(m_objectInfo.emplace_back());
		objectInfo->objectID = objectIndex;
		newObject->SetObjectInfo(objectInfo);

		newObject->SetMeshChangedCallback([this]() { this->MeshChanged(); });
		newObject->SetTransformChangedCallback([this]() { this->TransformChanged(); });

		m_onAddObject(newObject);

		return newObject;
	}

	const ObjectInfo& MObjectRegistry::GetObjectInfo(const size_t index) const
	{
		ASSERT(m_objectInfo.size() > index, "ERROR!!! Out of bounds access.");
		return m_objectInfo[index];
	}

	std::vector<ObjectInfo> MObjectRegistry::GetAllObjectInfoCopy() const
	{
		std::vector<ObjectInfo> copy = { m_objectInfo.begin(), m_objectInfo.end() };
		return copy;
	}
};