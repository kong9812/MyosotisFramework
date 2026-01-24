// Copyright (c) 2025 kong9812
#pragma once
#include "ClassPointer.h"
#include "UpdateData.h"
#include "Transform.h"
#include "MObject.h"

#include "AxisDrawCommand.h"

namespace MyosotisFW::System::Render
{
	class Gizmo
	{
	public:
		Gizmo() :
			m_enable(false),
			m_sortedAxes({}),
			m_selectedObject(nullptr),
			m_isHovered(false) {
		}
		~Gizmo() {}

		void Update(const UpdateData& updateData, const Camera::CameraBase_ptr& mainCamera);

		void SetSelectObject(const MObject_ptr& object) { m_selectedObject = object; }
		const std::vector<AxisDrawCommand>& GetGizmoAxisDrawCommand() const { return m_sortedAxes; }
		const bool IsEnable() const { return m_enable; }

	private:
		bool m_enable;
		MObject_ptr m_selectedObject;
		bool m_isHovered;

		std::vector<AxisDrawCommand> m_sortedAxes;
	};
	TYPEDEF_SHARED_PTR_ARGS(Gizmo);
}