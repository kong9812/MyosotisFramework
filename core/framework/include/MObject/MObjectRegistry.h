// Copyright (c) 2025 kong9812
#pragma once
#include <vector>
#include <deque>
#include "ClassPointer.h"
#include "ObjectInfo.h"

namespace MyosotisFW
{
	class MObject;
	TYPEDEF_SHARED_PTR_FWD(MObject);

	class MObjectRegistry
	{
	public:
		MObjectRegistry() {}
		~MObjectRegistry() {}

		MObject_ptr CreateNewObject();

		const ObjectInfo& GetObjectInfo(const size_t index) const;
		std::vector<ObjectInfo> GetAllObjectInfoCopy() const;

	private:
		std::vector<MObject_ptr> m_object;
		std::deque<ObjectInfo> m_objectInfo;
	};
	TYPEDEF_SHARED_PTR(MObjectRegistry);
};