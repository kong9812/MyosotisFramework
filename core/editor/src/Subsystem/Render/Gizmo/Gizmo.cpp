// Copyright (c) 2025 kong9812
#include "Gizmo.h"
#include "MObject.h"
#include "Camera.h"
#include "AxisConfig.h"
#include "Collision.h"

#include "Logger.h"

namespace {
	constexpr std::array<MyosotisFW::AxisConfig, 3> g_axesConfig = { {
		{ glm::vec3(0.0f, 0.0f, -90.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f) }, // X軸: 赤
		{ glm::vec3(0.0f, 0.0f, 0.0f),   glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f) }, // Y軸: 緑
		{ glm::vec3(90.0f, 0.0f, 0.0f),  glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f) }  // Z軸: 青
	} };
	constexpr float g_axesOffsetDistance = 0.1f;
}

namespace MyosotisFW::System::Render
{
	void Gizmo::Update(const UpdateData& updateData, const Camera::CameraBase_ptr& mainCamera)
	{
		m_enable = (m_selectedObject != nullptr);

		if (m_selectedObject)
		{
			glm::mat4 baseModelMatrix = glm::mat4(0.0f);
			baseModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(m_selectedObject->GetPos()));
			baseModelMatrix = glm::rotate(baseModelMatrix, glm::radians(m_selectedObject->GetRot().x), glm::vec3(1.0f, 0.0f, 0.0f));
			baseModelMatrix = glm::rotate(baseModelMatrix, glm::radians(m_selectedObject->GetRot().y), glm::vec3(0.0f, 1.0f, 0.0f));
			baseModelMatrix = glm::rotate(baseModelMatrix, glm::radians(m_selectedObject->GetRot().z), glm::vec3(0.0f, 0.0f, 1.0f));
			baseModelMatrix = glm::scale(baseModelMatrix, glm::vec3(m_selectedObject->GetScale()));

			glm::vec3 cameraPos = mainCamera->GetCameraPos();
			m_sortedAxes.clear();

			for (const AxisConfig& config : g_axesConfig)
			{
				glm::mat4 model = baseModelMatrix;
				model = glm::translate(model, config.direction * g_axesOffsetDistance);
				model = glm::rotate(model, glm::radians(config.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::rotate(model, glm::radians(config.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, glm::radians(config.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

				glm::vec4 color = config.color;
				float dist = glm::distance(cameraPos, glm::vec3(model[1]));
				m_sortedAxes.push_back({ model, color, dist });
			}
			std::sort(m_sortedAxes.begin(), m_sortedAxes.end(), [](const AxisDrawCommand& a, const AxisDrawCommand& b)
				{
					return a.distance > b.distance;
				});
		}
	}
}