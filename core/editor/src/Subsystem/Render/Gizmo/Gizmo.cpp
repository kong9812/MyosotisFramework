// Copyright (c) 2025 kong9812
#include "Gizmo.h"
#include "iglfw.h"
#include "MObject.h"
#include "Camera.h"
#include "AxisConfig.h"
#include "Collision.h"

#include "Logger.h"

namespace {
	constexpr uint32_t g_axesCount = static_cast<uint32_t>(MyosotisFW::System::Render::Gizmo::GizmoAxes::Count);
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

	// ピック閾値
	constexpr float g_pickMul = 2.0f;			// 取りやすさ係数 (太めに拾う)
	constexpr float g_pickRadiusPx = 6.0f;		// カーソルから何px以内なら拾う
	constexpr float g_arrowTotalSize = 1.0f;	// ギズモのサイズ
	// createArrow のデフォルトサイズ割合
	constexpr float cylinderHeightRatio = 0.75f;	// [割合] 軸の高さ			(75%)
	constexpr float cylinderRadiusRatio = 0.05f;	// [割合] 軸の半径			(5%)
	constexpr float coneHeightRatio = 0.25f;		// [割合] コーンの高さ		(25%)
	constexpr float coneRadiusRatio = 0.15f;		// [割合] コーンの半径		(15%)
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
		// マウス左ボタンが外した
		if (leftReleasedThisFrame)
		{
			if (m_objectMovedCallback) m_objectMovedCallback();		// 位置確定
			m_gizmoAxesData.isUsing = false;						// 全体の利用状態リセット
			m_gizmoAxesData.axes = { false, false, false };			// 各軸の利用状態解除
		}

		// カメラなし・オブジェクト選択してない -> 処理しない
		if ((!m_selectedObject) || (!mainCamera)) return;

		// レイ作成 (MousePosの near->far)
		const Ray ray = mainCamera->GetRay(updateData.mousePos);

		// オブジェクトの基準行列
		glm::mat4 baseModelMatrix = glm::mat4(1.0f);
		baseModelMatrix = glm::translate(baseModelMatrix, glm::vec3(m_selectedObject->GetPos()));
		baseModelMatrix = glm::rotate(baseModelMatrix, glm::radians(m_selectedObject->GetRot().x), glm::vec3(1.0f, 0.0f, 0.0f));
		baseModelMatrix = glm::rotate(baseModelMatrix, glm::radians(m_selectedObject->GetRot().y), glm::vec3(0.0f, 1.0f, 0.0f));
		baseModelMatrix = glm::rotate(baseModelMatrix, glm::radians(m_selectedObject->GetRot().z), glm::vec3(0.0f, 0.0f, 1.0f));
		//baseModelMatrix = glm::scale(baseModelMatrix, glm::vec3(m_selectedObject->GetScale()));

		const glm::vec3 cameraPos = mainCamera->GetCameraPos();
		const glm::vec3 originWorld = glm::vec3(baseModelMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));	// ギズモの中心

		// [初回Press] 各軸の利用状態解除
		const bool isFirstPress = leftPressedThisFrame && (!m_gizmoAxesData.isUsing);
		if (isFirstPress)
		{
			m_gizmoAxesData.axes = { false, false, false };
		}

		// 後で使うキャッシュ
		std::array<glm::vec3, g_axesCount> axisDirCache{};	// 軸のワールド方向
		std::array<float, g_axesCount> distToCamCache{};	// 描画順ソート用

		for (uint32_t axes = 0; axes < g_axesCount; axes++)
		{
			const AxisConfig& config = g_axesConfig[axes];

			// ギズモのmodel行列作成
			glm::mat4 model = baseModelMatrix;
			model = glm::translate(model, config.direction * g_axesOffsetDistance);
			model = glm::rotate(model, glm::radians(config.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::rotate(model, glm::radians(config.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::rotate(model, glm::radians(config.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

			// 矢印線分を作成
			const glm::vec3 p0 = glm::vec3(model * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
			const glm::vec3 p1 = glm::vec3(model * glm::vec4(0.0f, g_arrowTotalSize, 0.0f, 1.0f));
			const glm::vec3 mid = (p0 + p1) * 0.5f;

			// 画面px基準で拾う半径の計算
			float thresholdByPx = 0.0f;
			if (updateData.screenSize.y > 1.0f)
			{
				const float distToCam = glm::distance(cameraPos, mid);						// カメラ <-> 線分中央 の距離
				const float proj11 = std::abs(mainCamera->GetProjectionMatrix()[1][1]);		// FOV情報を取り出す (1/tan(fovY/2))
				const float tanHalfFovY = (proj11 > 1e-6f) ? (1.0f / proj11) : 1.0f;		// tan(fovY/2)の逆算
				// 画面全体の高さ: (2 * distToCam * tanHalfFovY)
				// 1ピクセルあたりのワールド長さの計算 (Y軸)↓
				const float worldPrePixel = (2.0f * distToCam * tanHalfFovY) / updateData.screenSize.y;
				thresholdByPx = worldPrePixel * g_pickRadiusPx;	// px半径 -> ワールド半径
			}

			float u = 0.0f;	// 線分上の度の位置が最短だったか 0:先頭 1:最後
			const float rayToSegmentDist = DistanceRayToSegment(ray.origin, ray.dir, p0, p1, u);	// 距離

			// コーンを太めに拾う準備
			const float coneT = glm::clamp((u - cylinderHeightRatio) / coneHeightRatio, 0.0f, 1.0f);
			const float radiusRatio = glm::mix(cylinderRadiusRatio, coneRadiusRatio, coneT);

			// メッシュ基準で拾う半径を計算
			const float worldLen = glm::distance(p0, p1);
			float thresholdByMesh = worldLen * cylinderRadiusRatio * g_pickMul;

			// px基準とメッシュ基準 大きい方(拾いやすい方)を使う
			const float threshold = std::max(thresholdByMesh, thresholdByPx);
			const bool hit = (rayToSegmentDist < threshold);

			// [初回Press] 軸を選択
			if (isFirstPress && hit)
			{
				m_gizmoAxesData.axes[axes] = true;
			}

			// キャッシュ
			axisDirCache[axes] = glm::normalize(p1 - p0);							// 軸のワールド方向
			distToCamCache[axes] = glm::distance(cameraPos, glm::vec3(model[3]));	// 描画ソート用

			// 色
			glm::vec4 color = config.normalColor;
			if (m_gizmoAxesData.isUsing)
			{
				// Drag中
				if (m_gizmoAxesData.axes[axes]) color = config.pressedColor;
			}
			else
			{
				// hover状態
				if (hit)
				{
					color = config.hoverColor;
					m_hovered = true;
				}
			}
			m_sortedAxes.push_back({ model, color, distToCamCache[axes] });
		}

		// [初回Press] ドラッグ開始
		if (isFirstPress)
		{
			//int32_t count = std::count_if(m_gizmoAxesData.axes.begin(), m_gizmoAxesData.axes.end(), [](bool b) {return (b == true); });
			int32_t count = 0;
			for (const bool b : m_gizmoAxesData.axes) if (b) count++;

			if (count > 0)
			{
				m_gizmoAxesData.isUsing = true;
				m_gizmoAxesData.type = GizmoType::Position;

				m_gizmoAxesData.dragStartObjectPosition = glm::vec3(m_selectedObject->GetPos());
				m_gizmoAxesData.dragPlanePoint = originWorld;

				for (uint32_t axes = 0; axes < g_axesCount; axes++)
				{
					m_gizmoAxesData.dragAxisDirectWorld[axes] = m_gizmoAxesData.axes[axes] ? axisDirCache[axes] : glm::vec3(0.0f);
				}

				glm::vec3 viewDir = glm::normalize(originWorld - cameraPos);	// カメラ -> ギズモ の方向
				glm::vec3 planeNormal = glm::vec3(0.0f);						// 移動用の平面法線

				if (count == 1)	// 軸数: 1
				{
					// 選ばれた軸を抽出
					int32_t a0 = m_gizmoAxesData.axes[0] ? 0 : (m_gizmoAxesData.axes[1] ? 1 : 2);
					const glm::vec3 axisDir = m_gizmoAxesData.dragAxisDirectWorld[a0];

					// スカラー: glm::dot(viewDir, axisDir)
					// 軸に沿ったベクトル: axisDir * dot
					// 軸に直交する成分 ↓
					planeNormal = viewDir - axisDir * glm::dot(viewDir, axisDir);

					// viewDir と planeNormal がほぼ平行 (軸の両端から見る時)
					if (glm::dot(planeNormal, planeNormal) < 1e-6f)
					{
						glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
						// 外積に逃げる
						if (std::abs(glm::dot(axisDir, up)) > 0.99f) up = glm::vec3(1.0f, 0.0f, 0.0f);
						planeNormal = glm::cross(axisDir, up);
					}
				}
				else if (count == 2)	// 軸数: 2
				{
					// 選ばれた軸を抽出
					int32_t a0 = -1;
					int32_t a1 = -1;
					for (uint32_t axes = 0; axes < g_axesCount; axes++)
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

					// いつもの法線計算 = 2軸の外積
					planeNormal = glm::cross(m_gizmoAxesData.dragAxisDirectWorld[a0], m_gizmoAxesData.dragAxisDirectWorld[a1]);

					// 2軸が平行 -> viewDirに逃げる
					if (glm::dot(planeNormal, planeNormal) < 1e-6f)
					{
						planeNormal = viewDir;
					}
				}
				else  // 軸数: 3
				{
					// 画面平面移動
					planeNormal = viewDir;
				}

				// 正規化
				m_gizmoAxesData.dragPlaneNormal = glm::normalize(planeNormal);

				glm::vec3 startHitPos{};
				if (!RayPlaneIntersect(ray, m_gizmoAxesData.dragPlaneNormal, m_gizmoAxesData.dragPlanePoint, startHitPos))	// 平面とレイの交点
				{
					// 交点がない 選択解除
					m_gizmoAxesData.isUsing = false;
					m_gizmoAxesData.axes = { false, false, false };
				}
				else
				{
					// 移動開始位置をセット
					m_gizmoAxesData.dragStartHitPosition = startHitPos;
				}
			}
		}

		// ドラッグ中
		if ((m_gizmoAxesData.isUsing) && (leftDown))
		{
			glm::vec3 hitPos{};
			if (RayPlaneIntersect(ray, m_gizmoAxesData.dragPlaneNormal, m_gizmoAxesData.dragPlanePoint, hitPos))	// 平面とレイの交点
			{
				const glm::vec3 delta = hitPos - m_gizmoAxesData.dragStartHitPosition;	// 押した時で記録された交点との差分

				// 移動軸数の抽出
				int32_t count = 0;
				for (const bool b : m_gizmoAxesData.axes) if (b) count++;

				// 移動量計算
				glm::vec3 move = glm::vec3(0.0f);
				if (count == 3)	// 3軸移動
				{
					// 自由移動
					move = delta;
				}
				else
				{
					for (uint32_t axes = 0; axes < g_axesCount; axes++)
					{
						if (!m_gizmoAxesData.axes[axes]) continue;

						// 選択軸の移動量を追加
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