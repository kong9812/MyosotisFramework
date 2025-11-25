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
		MObjectRegistry() :
			m_object(),
			m_objectInfo(),
			m_meshChanged(true),
			m_transformChanged(true) {
		}
		~MObjectRegistry() {}

		MObject_ptr CreateNewObject();

		const ObjectInfo& GetObjectInfo(const size_t index) const;
		std::vector<ObjectInfo> GetAllObjectInfoCopy() const;

		bool IsMeshChanged() const { return m_meshChanged; }
		bool IsTransformChanged() const { return m_transformChanged; }

		void MeshChanged() { m_meshChanged = true; }
		void TransformChanged() { m_transformChanged = true; }
		void ResetChangeFlags() { m_meshChanged = false; m_transformChanged = false; }

	private:
		std::vector<MObject_ptr> m_object;
		std::deque<ObjectInfo> m_objectInfo;

		bool m_meshChanged;
		bool m_transformChanged;
	};
	TYPEDEF_SHARED_PTR(MObjectRegistry);
};