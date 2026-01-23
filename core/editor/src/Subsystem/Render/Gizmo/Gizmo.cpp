// Copyright (c) 2025 kong9812
#include "Gizmo.h"
#include "MObject.h"

namespace MyosotisFW::System::Render
{
	void Gizmo::Update(const UpdateData& updateData)
	{
		m_enable = (m_selectedObject != nullptr);

		if (m_selectedObject)
		{
			m_transform.pos = m_selectedObject->GetPos();
		}
	}
}