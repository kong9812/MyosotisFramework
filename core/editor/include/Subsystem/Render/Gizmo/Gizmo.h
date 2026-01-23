// Copyright (c) 2025 kong9812
#pragma once
#include "ClassPointer.h"
#include "UpdateData.h"
#include "Transform.h"
#include "MObject.h"

namespace MyosotisFW::System::Render
{
	class Gizmo
	{
	public:
		Gizmo() :
			m_enable(false),
			m_transform(),
			m_selectedObject(nullptr) {
		}
		~Gizmo() {}

		void Update(const UpdateData& updateData);

		void SetSelectObject(const MObject_ptr& object) { m_selectedObject = object; }
		const Transform& GetGizmoTransform() const { return m_transform; }
		const bool IsEnable() const { return m_enable; }

	private:
		bool m_enable;
		Transform m_transform;
		MObject_ptr m_selectedObject;

	};
	TYPEDEF_SHARED_PTR_ARGS(Gizmo);
}