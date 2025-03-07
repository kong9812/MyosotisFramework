// Copyright (c) 2025 kong9812
#include "Camera.h"
#include "AppInfo.h"
#include "iimgui.h"

namespace MyosotisFW::System::Render::Camera
{
	CameraBase::CameraBase() : ObjectBase()
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

		m_name = "カメラ";
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

	//    void CameraBase::BindDebugGUIElement()
	//    {
	//#ifndef RELEASE
	//        ImGui::Begin("Camera Pos",
	//            (bool*)true,
	//            ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysAutoResize |
	//            ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar);
	//        ImGui::Text("pos: %.2f %.2f %.2f", m_cameraPos.x, m_cameraPos.y, m_cameraPos.z);
	//        ImGui::Text("front: %.2f %.2f %.2f", m_cameraFront.x, m_cameraFront.y, m_cameraFront.z);
	//        ImGui::End();
	//#endif // !RELEASE
	//    }

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

	void CameraBase::Deserialize(const rapidjson::Value& doc, const std::function<void(ObjectType, const rapidjson::Value&)>& createObject)
	{
		__super::Deserialize(doc, createObject);

		DeserializeVec3FromJson<glm::vec3>("cameraPos", m_cameraPos, doc);
		DeserializeVec3FromJson<glm::vec3>("cameraLookAt", m_cameraLookAt, doc);
		DeserializeVec3FromJson<glm::vec3>("cameraFront", m_cameraFront, doc);
		DeserializeVec3FromJson<glm::vec3>("cameraUp", m_cameraUp, doc);
		DeserializeVec3FromJson<glm::vec3>("cameraRight", m_cameraRight, doc);

		m_cameraFov = doc["cameraFov"].GetFloat();
		m_cameraFar = doc["cameraFar"].GetFloat();
		m_cameraNear = doc["cameraNear"].GetFloat();
	}
}
