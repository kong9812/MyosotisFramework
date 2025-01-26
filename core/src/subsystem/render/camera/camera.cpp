// Copyright (c) 2025 kong9812
#include "camera.h"
#include "appInfo.h"
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

    float CameraBase::GetDistance(glm::vec3 pos) const
    {
        return glm::distance(m_cameraPos, pos);
    }

    glm::vec3 CameraBase::GetFrontPos(float distance) const
    {
        return m_cameraPos + (m_cameraFront * distance);
    }

    void CameraBase::BindDebugGUIElement()
    {
#ifndef RELEASE
        ImGui::Begin("Camera Pos",
            (bool*)true,
            ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar);
        ImGui::Text("pos: %.2f %.2f %.2f", m_cameraPos.x, m_cameraPos.y, m_cameraPos.z);
        ImGui::Text("front: %.2f %.2f %.2f", m_cameraFront.x, m_cameraFront.y, m_cameraFront.z);
        ImGui::End();
#endif // !RELEASE
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

    void CameraBase::Deserialize(const rapidjson::Value& doc, std::function<void(ObjectType, const rapidjson::Value&)> createObject)
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
