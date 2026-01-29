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

		// マウス左ボタン状態
		auto mouseBtn = updateData.mouseButtonActions.find(GLFW_MOUSE_BUTTON_LEFT);
		const bool leftPressedThisFrame = (mouseBtn != updateData.mouseButtonActions.end() && (mouseBtn->second == GLFW_PRESS));
		const bool leftReleasedThisFrame = (mouseBtn != updateData.mouseButtonActions.end() && (mouseBtn->second == GLFW_RELEASE));
		const bool leftDown = (mouseBtn != updateData.mouseButtonActions.end() && ((mouseBtn->second == GLFW_PRESS) || (mouseBtn->second == GLFW_REPEAT)));
		if (leftReleasedThisFrame)
		{
			if (m_objectMovedCallback) m_objectMovedCallback();
			m_gizmoAxesData.isUsing = false;
			m_gizmoAxesData.axes = { false, false, false };
		}

		if ((!m_selectedObject) || (!mainCamera)) return;

		const Ray ray = mainCamera->GetRay(updateData.mousePos);

		glm::mat4 baseModelMatrix = glm::mat4(1.0f);
		baseModelMatrix = glm::translate(baseModelMatrix, glm::vec3(m_selectedObject->GetPos()));
		baseModelMatrix = glm::rotate(baseModelMatrix, glm::radians(m_selectedObject->GetRot().x), glm::vec3(1.0f, 0.0f, 0.0f));
		baseModelMatrix = glm::rotate(baseModelMatrix, glm::radians(m_selectedObject->GetRot().y), glm::vec3(0.0f, 1.0f, 0.0f));
		baseModelMatrix = glm::rotate(baseModelMatrix, glm::radians(m_selectedObject->GetRot().z), glm::vec3(0.0f, 0.0f, 1.0f));
		baseModelMatrix = glm::scale(baseModelMatrix, glm::vec3(m_selectedObject->GetScale()));

		const glm::vec3 cameraPos = mainCamera->GetCameraPos();
		const glm::vec3 originWorld = glm::vec3(baseModelMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

		// ピック閾値
		constexpr float pickMul = 2.0f;				//取りやすさ係数
		constexpr float pickRadiusPx = 6.0f;		// 画面上でだいたい何ピクセル以内を拾うか
		constexpr float arrowTotalSize = 1.0f;		// 矢印ローカル長さ

		// createArrow比率
		constexpr float shaftRadiusRatio = 0.05f;	// 軸(5%)
		constexpr float coneRadiusRatio = 0.15f;	// コーン(15%)

		const bool isFirstPress = leftPressedThisFrame && (!m_gizmoAxesData.isUsing);
		if (isFirstPress)
		{
			m_gizmoAxesData.axes = { false, false, false };
		}

		// 初回Press後に使うキャッシュ
		std::array<glm::vec3, 3> p0Cache{}, p1Cache{}, axisDirCache{};
		std::array<glm::mat4, 3> modelCache{};
		std::array<float, 3> distToCamCache{};

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

			float u = 0.0f;	// 使わないかも...
			const float rayToSegmentDist = DistanceRayToSegment(ray.origin, ray.dir, p0, p1, u);

			// 先端(コーン)側は太めに拾う
			const float coneT = glm::clamp((u - 0.75f) / 0.25f, 0.0f, 1.0f);
			const float radiusRatio = glm::mix(shaftRadiusRatio, coneRadiusRatio, coneT);

			//矢印 world size
			const float worldLen = glm::distance(p0, p1);
			float thresholdByMesh = worldLen * shaftRadiusRatio * pickMul;

			const float threshold = std::max(thresholdByMesh, thresholdByPx);
			const bool hit = (rayToSegmentDist < threshold);

			// 初回Pressでヒットした軸を全選択
			if (isFirstPress && hit)
			{
				m_gizmoAxesData.axes[axes] = true;
			}

			// キャッシュ
			modelCache[axes] = model;
			p0Cache[axes] = p0;
			p1Cache[axes] = p1;
			axisDirCache[axes] = glm::normalize(p1 - p0);
			distToCamCache[axes] = glm::distance(cameraPos, glm::vec3(model[3]));

			// 色
			glm::vec4 color = config.normalColor;
			if (m_gizmoAxesData.isUsing)
			{
				if (m_gizmoAxesData.axes[axes]) color = config.pressedColor;
			}
			else
			{
				if (hit)
				{
					color = config.hoverColor;
					m_hovered = true;
				}
			}
			m_sortedAxes.push_back({ model, color, distToCamCache[axes] });
		}

		// 初回Press ドラッグ開始
		if (isFirstPress)
		{
			//int count = std::count_if(m_gizmoAxesData.axes.begin(), m_gizmoAxesData.axes.end(), [](bool b) {return (b == true); });
			int count = 0;
			for (const bool b : m_gizmoAxesData.axes) if (b) count++;

			if (count > 0)
			{
				m_gizmoAxesData.isUsing = true;
				m_gizmoAxesData.type = GizmoType::Position;

				m_gizmoAxesData.dragStartObjectPosition = glm::vec3(m_selectedObject->GetPos());
				m_gizmoAxesData.dragPlanePoint = originWorld;

				for (uint32_t axes = 0; axes < static_cast<uint32_t>(GizmoAxes::Count); axes++)
				{
					m_gizmoAxesData.dragAxisDirectWorld[axes] = m_gizmoAxesData.axes[axes] ? axisDirCache[axes] : glm::vec3(0.0f);
				}

				glm::vec3 viewDir = glm::normalize(originWorld - cameraPos);
				glm::vec3 planeNormal = glm::vec3(0.0f);

				if (count == 1)
				{
					int a0 = m_gizmoAxesData.axes[0] ? 0 : (m_gizmoAxesData.axes[1] ? 1 : 2);
					const glm::vec3 axisDir = m_gizmoAxesData.dragAxisDirectWorld[a0];

					planeNormal = viewDir - axisDir * glm::dot(viewDir, axisDir);

					if (glm::dot(planeNormal, planeNormal) < 1e-6f)
					{
						glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
						if (std::abs(glm::dot(axisDir, up)) > 0.99f) up = glm::vec3(1.0f, 0.0f, 0.0f);
						planeNormal = glm::cross(axisDir, up);
					}
				}
				else if (count == 2)
				{
					int a0 = -1;
					int a1 = -1;
					for (uint32_t axes = 0; axes < static_cast<uint32_t>(GizmoAxes::Count); axes++)
					{
						if (m_gizmoAxesData.axes[axes])
						{
							if (a0 < 0)
							{
								a0 = axes;
							}
							else
							{
								a1 = axes;
							}
						}
					}

					planeNormal = glm::cross(m_gizmoAxesData.dragAxisDirectWorld[a0], m_gizmoAxesData.dragAxisDirectWorld[a1]);

					if (glm::dot(planeNormal, planeNormal) < 1e-6f)
					{
						planeNormal = viewDir;
					}
				}
				else  // count == 3 全部だね
				{
					// 画面の平面移動だねでOK
					planeNormal = viewDir;
				}

				m_gizmoAxesData.dragPlaneNormal = glm::normalize(planeNormal);

				glm::vec3 startHitPos{};
				if (!RayPlaneIntersect(ray, m_gizmoAxesData.dragPlaneNormal, m_gizmoAxesData.dragPlanePoint, startHitPos))
				{
					// 交差点とれないならキャッシュを使う
					m_gizmoAxesData.isUsing = false;
					m_gizmoAxesData.axes = { false, false, false };
				}
				else
				{
					m_gizmoAxesData.dragStartHitPosition = startHitPos;
				}
			}
		}

		// ドラッグ中
		if ((m_gizmoAxesData.isUsing) && (leftDown))
		{
			glm::vec3 hitPos{};
			if (RayPlaneIntersect(ray, m_gizmoAxesData.dragPlaneNormal, m_gizmoAxesData.dragPlanePoint, hitPos))
			{
				const glm::vec3 delta = hitPos - m_gizmoAxesData.dragStartHitPosition;

				int count = 0;
				for (const bool b : m_gizmoAxesData.axes) if (b) count++;

				glm::vec3 move = glm::vec3(0.0f);
				if (count == 3)
				{
					// 自由移動
					move = delta;
				}
				else
				{
					for (uint32_t axes = 0; axes < static_cast<uint32_t>(GizmoAxes::Count); axes++)
					{
						if (!m_gizmoAxesData.axes[axes]) continue;
						const glm::vec3 axisDir = m_gizmoAxesData.dragAxisDirectWorld[axes];
						move += axisDir * glm::dot(delta, axisDir);
					}
				}
				m_selectedObject->SetPos(m_gizmoAxesData.dragStartObjectPosition + move);
			}
		}

		std::sort(m_sortedAxes.begin(), m_sortedAxes.end(), [](const AxisDrawCommand& a, const AxisDrawCommand& b)
			{
				return a.distance > b.distance;
			});
	}
}