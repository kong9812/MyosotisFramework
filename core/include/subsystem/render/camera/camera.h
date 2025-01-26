// Copyright (c) 2025 kong9812
#pragma once
#include <string>

#include "objectCast.h"
#include "iglm.h"
#include "objectBase.h"
#include "classPointer.h"
#include "vkStruct.h"

namespace MyosotisFW::System::Render::Camera
{
    class CameraBase : public ObjectBase
    {
    public:
        CameraBase();
        virtual ~CameraBase() = default;

        virtual glm::mat4 GetViewMatrix() const;
        virtual glm::mat4 GetProjectionMatrix() const;
        
        virtual const ObjectType GetObjectType() const override { return ObjectType::Undefined; }

        void SetAspectRadio(float aspectRadio) { m_aspectRadio = aspectRadio; }

        float GetDistance(glm::vec3 pos) const;
        glm::vec3 GetFrontPos(float distance) const;

        glm::vec3 GetCameraPos() const { return m_cameraPos; }

        virtual void Update(const Utility::Vulkan::Struct::UpdateData& updateData) {}
        void BindCommandBuffer(VkCommandBuffer commandBuffer) override {}  // 必要ない
        virtual void BindDebugGUIElement() override;

        virtual rapidjson::Value Serialize(rapidjson::Document::AllocatorType& allocator) const override;
        virtual void Deserialize(const rapidjson::Value& doc, std::function<void(ObjectType, const rapidjson::Value&)> createObject) override;
    protected:
        // カメラ位置
        glm::vec3 m_cameraPos;
        // カメラ視点
        glm::vec3 m_cameraLookAt;
        // カメラ　前ベクトル
        glm::vec3 m_cameraFront;
        // カメラ　上ベクトル
        glm::vec3 m_cameraUp;
        // カメラ　右ベクトル
        glm::vec3 m_cameraRight;
        // カメラ FOV
        float m_cameraFov;
        // カメラ Far
        float m_cameraFar;
        // カメラ Near
        float m_cameraNear;
        // アスペクト比
        float m_aspectRadio;
    };
    TYPEDEF_SHARED_PTR(CameraBase)
    OBJECT_CAST_FUNCTION(CameraBase)
}