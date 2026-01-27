// Copyright (c) 2025 kong9812
#pragma once
#include <array>
#include "iglm.h"
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
			m_hovered(false),
			m_sortedAxes({}),
			m_selectedObject(nullptr) {
		}
		~Gizmo() {}

		void Update(const UpdateData& updateData, const Camera::CameraBase_ptr& mainCamera);

		void SetSelectObject(const MObject_ptr& object) { m_selectedObject = object; }
		const std::vector<AxisDrawCommand>& GetGizmoAxisDrawCommand() const { return m_sortedAxes; }
		const bool IsEnable() const { return m_enable; }
		const bool IsHovered() const { return m_hovered; }

	private:
		bool m_enable;
		bool m_hovered;
		MObject_ptr m_selectedObject;

		std::vector<AxisDrawCommand> m_sortedAxes;

	private:
		enum class GizmoType : uint32_t
		{
			Position = 0,
			Rotation,
			Scale,
		};
		enum class GizmoAxes : uint32_t
		{
			X = 0,
			Y,
			Z,
			Count,
		};

		struct {
			bool isUsing = false;
			GizmoType type = GizmoType::Position;
			std::array<bool, static_cast<uint32_t>(GizmoAxes::Count)> axes = { false, false, false };
			glm::vec3 lastPos = glm::vec3(0.0f);
			float distance = 0.0f;

		}m_gizmoAxesData;
	};
	TYPEDEF_SHARED_PTR_ARGS(Gizmo);
}