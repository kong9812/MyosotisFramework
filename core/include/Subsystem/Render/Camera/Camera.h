// Copyright (c) 2025 kong9812
#pragma once
#include <string>

#include "ObjectCast.h"
#include "iglm.h"
#include "ObjectBase.h"
#include "ClassPointer.h"
#include "Structs.h"

namespace MyosotisFW::System::Render::Camera
{
	class CameraBase : public ObjectBase
	{
	public:
		CameraBase();
		virtual ~CameraBase() = default;

		virtual glm::mat4 GetViewMatrix() const;
		virtual glm::mat4 GetProjectionMatrix() const;
		virtual glm::vec3 GetWorldPos(const glm::vec2& pos, const float& distance) const;

		virtual const ObjectType GetObjectType() const override { return ObjectType::Undefined; }

		void UpdateScreenSize(const glm::vec2& size);

		float GetDistance(const glm::vec3& pos) const;
		glm::vec3 GetFrontPos(const float& distance) const;

		glm::vec3 GetCameraPos() const { return m_cameraPos; }

		virtual void Update(const UpdateData& updateData) {}
		//virtual void BindDebugGUIElement() override;

		virtual rapidjson::Value Serialize(rapidjson::Document::AllocatorType& allocator) const override;
		virtual void Deserialize(const rapidjson::Value& doc, const std::function<void(ObjectType, const rapidjson::Value&)>& createObject) override;
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
		// 画面サイズ
		glm::vec2 m_screenSize;
	};
	TYPEDEF_SHARED_PTR(CameraBase)
		OBJECT_CAST_FUNCTION(CameraBase)
}