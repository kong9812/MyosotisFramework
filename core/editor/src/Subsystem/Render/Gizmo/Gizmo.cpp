// Copyright (c) 2025 kong9812
#include "Gizmo.h"
#include "iglfw.h"
#include "MObject.h"
#include "Camera.h"
#include "AxisConfig.h"
#include "Collision.h"

#include "Logger.h"

namespace {
	constexpr std::array<MyosotisFW::AxisConfig, 3> g_axesConfig =
	{ {
		{// X軸: 赤
			glm::vec3(0.0f, 0.0f, -90.0f),		// rot
			glm::vec3(1.0f, 0.0f, 0.0f),		// dir
			glm::vec4(1.0f, 0.2f, 0.2f, 1.0f),	// normal color
			glm::vec4(1.0f, 0.7f, 0.7f, 1.0f),	// hover color
			glm::vec4(1.0f, 0.85f, 0.1f, 1.0f)	// pressed color
		},
		{// Y軸: 緑
			glm::vec3(0.0f, 0.0f, 0.0f),		// rot
			glm::vec3(0.0f, 1.0f, 0.0f),		// dir
			glm::vec4(0.2f, 1.0f, 0.2f, 1.0f),	// normal color
			glm::vec4(0.7f, 1.0f, 0.7f, 1.0f),	// hover color
			glm::vec4(1.0f, 0.85f, 0.1f, 1.0f)	// pressed color
		},
		{// Z軸: 青
			glm::vec3(90.0f, 0.0f, 0.0f),		// rot
			glm::vec3(0.0f, 0.0f, 1.0f),		// dir
			glm::vec4(0.2f, 0.55f, 1.0f, 1.0f),	// normal color
			glm::vec4(0.7f, 0.85f, 1.0f, 1.0f),	// hover color
			glm::vec4(1.0f, 0.85f, 0.1f, 1.0f)	// pressed color
		}
	} };
	constexpr float g_axesOffsetDistance = 0.1f;
}

namespace MyosotisFW::System::Render
{
	void Gizmo::Update(const UpdateData& updateData, const Camera::CameraBase_ptr& mainCamera)
	{
		m_sortedAxes.clear();
		m_enable = (m_selectedObject != nullptr);
		m_hovered = false;

		auto mouseBtn = updateData.mouseButtonActions.find(GLFW_MOUSE_BUTTON_LEFT);
		if (mouseBtn != updateData.mouseButtonActions.end())
		{
			if (mouseBtn->second == GLFW_RELEASE)
			{
				m_gizmoAxesData.isUsing = false;
				m_gizmoAxesData.axes = { false, false, false };
			}
		}

		if (m_selectedObject)
		{
			const Ray ray = mainCamera->GetRay(updateData.mousePos);

			glm::mat4 baseModelMatrix = glm::mat4(1.0f);
			baseModelMatrix = glm::translate(baseModelMatrix, glm::vec3(m_selectedObject->GetPos()));
			baseModelMatrix = glm::rotate(baseModelMatrix, glm::radians(m_selectedObject->GetRot().x), glm::vec3(1.0f, 0.0f, 0.0f));
			baseModelMatrix = glm::rotate(baseModelMatrix, glm::radians(m_selectedObject->GetRot().y), glm::vec3(0.0f, 1.0f, 0.0f));
			baseModelMatrix = glm::rotate(baseModelMatrix, glm::radians(m_selectedObject->GetRot().z), glm::vec3(0.0f, 0.0f, 1.0f));
			baseModelMatrix = glm::scale(baseModelMatrix, glm::vec3(m_selectedObject->GetScale()));

			glm::vec3 cameraPos = mainCamera->GetCameraPos();

			// 計算用閾値
			constexpr float shaftRadiusRatio = 0.05f;	// 軸を細く(5%)
			constexpr float pickMul = 2.0f;				//取りやすさ係数
			constexpr float pickRadiusPx = 6.0f;		// 画面上でだいたい何ピクセル以内を拾うか
			constexpr float arrowTotalSize = 1.0f;		// 矢印ローカル長さ
			const glm::vec3 originWorld = glm::vec3(baseModelMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

			bool isFirstPress = false;
			auto mouseBtn = updateData.mouseButtonActions.find(GLFW_MOUSE_BUTTON_LEFT);
			if (mouseBtn != updateData.mouseButtonActions.end() && mouseBtn->second == GLFW_PRESS)
			{
				if (!m_gizmoAxesData.isUsing) isFirstPress = true;
			}

			for (uint32_t axes = 0; axes < static_cast<uint32_t>(GizmoAxes::Count); axes++)
			{
				const AxisConfig& config = g_axesConfig[axes];
				glm::mat4 model = baseModelMatrix;
				model = glm::translate(model, config.direction * g_axesOffsetDistance);
				model = glm::rotate(model, glm::radians(config.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::rotate(model, glm::radians(config.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, glm::radians(config.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

				// 矢印線分を作成
				const glm::vec3 p0 = glm::vec3(model * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
				const glm::vec3 p1 = glm::vec3(model * glm::vec4(0.0f, arrowTotalSize, 0.0f, 1.0f));
				const glm::vec3 mid = (p0 + p1) * 0.5f;
				//矢印 world size
				const float worldLen = glm::distance(p0, p1);
				float thresholdByMesh = worldLen * shaftRadiusRatio * pickMul;
				// px基準
				float thresholdByPx = 0.0f;
				if (updateData.screenSize.y > 1.0f)
				{
					const float distToCam = glm::distance(cameraPos, mid);
					const float proj11 = std::abs(mainCamera->GetProjectionMatrix()[1][1]);
					const float tanHalfFovY = (proj11 > 1e-6f) ? (1.0f / proj11) : 1.0f;
					const float worldPrePixel = (2.0f * distToCam * tanHalfFovY) / updateData.screenSize.y;
					thresholdByPx = worldPrePixel * pickRadiusPx;
				}
				const float threshold = std::max(thresholdByMesh, thresholdByPx);

				float u = 0.0f;	// 使わないかも...
				const float rayToSegmentDist = DistanceRayToSegment(ray.origin, ray.dir, p0, p1, u);
				glm::vec4 color = config.normalColor;
				if (rayToSegmentDist < threshold)
				{
					if (!m_gizmoAxesData.isUsing || m_gizmoAxesData.axes[axes])
					{
						color = config.hoverColor;
						m_hovered = true;
					}

					if (isFirstPress)
					{
						m_gizmoAxesData.axes[axes] = true;
						m_gizmoAxesData.type = GizmoType::Position;

						m_gizmoAxesData.distance = mainCamera->GetDistance(glm::vec3(m_selectedObject->GetPos()));
						m_gizmoAxesData.lastPos = mainCamera->GetWorldPos(updateData.mousePos, m_gizmoAxesData.distance);

					}
				}

				float dist = glm::distance(cameraPos, glm::vec3(model[3]));
				m_sortedAxes.push_back({ model, color, dist });
			}
			if (isFirstPress)
			{
				for (bool a : m_gizmoAxesData.axes)
				{
					if (a)
					{
						m_gizmoAxesData.isUsing = true;
						break;
					}
				}
			}

			std::sort(m_sortedAxes.begin(), m_sortedAxes.end(), [](const AxisDrawCommand& a, const AxisDrawCommand& b)
				{
					return a.distance > b.distance;
				});

			if (m_gizmoAxesData.isUsing)
			{
				glm::vec3 newPos = mainCamera->GetWorldPos(updateData.mousePos, m_gizmoAxesData.distance);
				glm::vec3 movePos = newPos - m_gizmoAxesData.lastPos;
				m_gizmoAxesData.lastPos = newPos;

				movePos.x = m_gizmoAxesData.axes[static_cast<uint32_t>(GizmoAxes::X)] ? movePos.x : 0.0f;
				movePos.y = m_gizmoAxesData.axes[static_cast<uint32_t>(GizmoAxes::Y)] ? movePos.y : 0.0f;
				movePos.z = m_gizmoAxesData.axes[static_cast<uint32_t>(GizmoAxes::Z)] ? movePos.z : 0.0f;

				glm::vec3 oldPos = m_selectedObject->GetPos();
				m_selectedObject->SetPos(oldPos + movePos);

				std::string debugText =
					"OldPos: " + std::to_string(oldPos.x) + " " + std::to_string(oldPos.y) + " " + std::to_string(oldPos.z) + " " +
					"NewPos: " + std::to_string(newPos.x) + " " + std::to_string(newPos.y) + " " + std::to_string(newPos.z) + " " +
					"MovePos: " + std::to_string(movePos.x) + " " + std::to_string(movePos.y) + " " + std::to_string(movePos.z) + " ";
				Logger::Debug(debugText);
			}
		}
	}
}