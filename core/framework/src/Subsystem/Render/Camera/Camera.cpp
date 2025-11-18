// Copyright (c) 2025 kong9812
#include "Camera.h"
#include "AppInfo.h"
#include "iimgui.h"
#include "CameraData.h"

namespace MyosotisFW::System::Render::Camera
{
	CameraBase::CameraBase() : ComponentBase(),
		m_cameraPos(glm::vec3(0.0f)),
		m_cameraLookAt(glm::vec3(0.0f)),
		m_cameraFront(glm::vec3(0.0f)),
		m_cameraUp(glm::vec3(0.0f)),
		m_cameraRight(glm::vec3(0.0f)),
		m_cameraFov(0.0f),
		m_cameraFar(0.0f),
		m_cameraNear(0.0f),
		m_aspectRadio(0.0f),
		m_screenSize(glm::vec2(0.0f))
	{
		m_name = "CameraBase";
		initialize();
	}

	void CameraBase::ResetCamera()
	{
		initialize();
	}

	glm::mat4 CameraBase::GetViewMatrix() const
	{
		return glm::lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);
	}

	glm::mat4 CameraBase::GetProjectionMatrix() const
	{
		return glm::perspective(glm::radians(m_cameraFov), m_aspectRadio, m_cameraNear, m_cameraFar);
	}

	glm::vec3 CameraBase::GetWorldPos(const glm::vec2& pos, const float& distance) const
	{
		// 画面座標を正規化デバイス座標（NDC）に変換
		glm::vec2 ndcPos;
		ndcPos.x = pos.x / m_screenSize.x * 2.0f - 1.0f;
		ndcPos.y = 1.0f - pos.y / m_screenSize.y * 2.0f;

		// 投影・ビュー行列の逆行列を取得
		glm::mat4 invProjView = glm::inverse(GetProjectionMatrix() * GetViewMatrix());

		// クリップ空間の座標を定義
		// Z値を調整してカメラからの距離をコントロール (例: 0.5f → カメラの中間距離)
		glm::vec4 clipPos(ndcPos.x, ndcPos.y, distance, 1.0f);

		// クリップ座標からワールド座標へ変換
		glm::vec4 worldPos = invProjView * clipPos;

		// wで除算して正規化し、最終的なワールド座標を返す
		return glm::vec3(worldPos) / worldPos.w;
	}

	CameraData CameraBase::GetCameraData() const
	{
		CameraData data{};
		data.position = glm::vec4(m_cameraPos, 0.0f);
		data.projection = GetProjectionMatrix();
		data.view = GetViewMatrix();

		glm::mat4 vp = GetProjectionMatrix() * GetViewMatrix();
		glm::vec4 rowX = glm::vec4(vp[0][0], vp[1][0], vp[2][0], vp[3][0]);
		glm::vec4 rowY = glm::vec4(vp[0][1], vp[1][1], vp[2][1], vp[3][1]);
		glm::vec4 rowZ = glm::vec4(vp[0][2], vp[1][2], vp[2][2], vp[3][2]);
		glm::vec4 rowW = glm::vec4(vp[0][3], vp[1][3], vp[2][3], vp[3][3]);
		data.frustumPlanes[0] = normalizePlane(rowW + rowX); // Left
		data.frustumPlanes[1] = normalizePlane(rowW - rowX); // Right
		data.frustumPlanes[2] = normalizePlane(rowW + rowY); // Bottom
		data.frustumPlanes[3] = normalizePlane(rowW - rowY); // Top
		data.frustumPlanes[4] = normalizePlane(rowW + rowZ); // Near
		data.frustumPlanes[5] = normalizePlane(rowW - rowZ); // Far

		return data;
	}

	void CameraBase::UpdateScreenSize(const glm::vec2& size)
	{
		m_screenSize = size;
		m_aspectRadio = size.x / size.y;
	}

	float CameraBase::GetDistance(const glm::vec3& pos) const
	{
		return glm::distance(m_cameraPos, pos);
	}

	glm::vec3 CameraBase::GetFrontPos(const float& distance) const
	{
		return m_cameraPos + (m_cameraFront * distance);
	}

	rapidjson::Value CameraBase::Serialize(rapidjson::Document::AllocatorType& allocator) const
	{
		rapidjson::Value doc = __super::Serialize(allocator);

		SerializeVec3ToJson<glm::vec3>("cameraPos", m_cameraPos, doc, allocator);
		SerializeVec3ToJson<glm::vec3>("cameraLookAt", m_cameraLookAt, doc, allocator);
		SerializeVec3ToJson<glm::vec3>("cameraFront", m_cameraFront, doc, allocator);
		SerializeVec3ToJson<glm::vec3>("cameraUp", m_cameraUp, doc, allocator);
		SerializeVec3ToJson<glm::vec3>("cameraRight", m_cameraRight, doc, allocator);

		doc.AddMember("cameraFov", m_cameraFov, allocator);
		doc.AddMember("cameraFar", m_cameraFar, allocator);
		doc.AddMember("cameraNear", m_cameraNear, allocator);

		return doc;
	}

	void CameraBase::Deserialize(const rapidjson::Value& doc)
	{
		__super::Deserialize(doc);

		DeserializeVec3FromJson<glm::vec3>("cameraPos", m_cameraPos, doc);
		DeserializeVec3FromJson<glm::vec3>("cameraLookAt", m_cameraLookAt, doc);
		DeserializeVec3FromJson<glm::vec3>("cameraFront", m_cameraFront, doc);
		DeserializeVec3FromJson<glm::vec3>("cameraUp", m_cameraUp, doc);
		DeserializeVec3FromJson<glm::vec3>("cameraRight", m_cameraRight, doc);

		m_cameraFov = doc["cameraFov"].GetFloat();
		m_cameraFar = doc["cameraFar"].GetFloat();
		m_cameraNear = doc["cameraNear"].GetFloat();
	}

	void CameraBase::initialize()
	{
		// カメラ位置
		m_cameraPos = AppInfo::g_cameraPos;
		// カメラ視点
		m_cameraLookAt = AppInfo::g_cameraLookAt;
		// カメラ　前ベクトル
		m_cameraFront = AppInfo::g_cameraFront;
		// カメラ　上ベクトル
		m_cameraUp = AppInfo::g_cameraUp;
		// カメラ　右ベクトル
		m_cameraRight = AppInfo::g_cameraRight;
		// カメラ FOV
		m_cameraFov = AppInfo::g_cameraFov;
		// カメラ Far
		m_cameraFar = AppInfo::g_cameraFar;
		// カメラ Near
		m_cameraNear = AppInfo::g_cameraNear;

		// アスペクト比
		m_aspectRadio = static_cast<float>(AppInfo::g_windowWidth) / static_cast<float>(AppInfo::g_windowHeight);
	}
}
