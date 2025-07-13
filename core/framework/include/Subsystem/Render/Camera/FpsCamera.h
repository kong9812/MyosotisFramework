// Copyright (c) 2025 kong9812
#pragma once
#include "Camera.h"
#include "ClassPointer.h"
#include "ObjectCast.h"

namespace MyosotisFW::System::Render::Camera
{
	class FPSCamera : public CameraBase
	{
	public:
		FPSCamera();
		~FPSCamera() {};

		glm::mat4 GetViewMatrix() const override;
		glm::mat4 GetProjectionMatrix() const override;
		glm::vec3 GetWorldPos(const glm::vec2& pos, const float& distance) const override;

		virtual const ObjectType GetObjectType() const override { return ObjectType::FPSCamera; }

		// 一時停止などマウス座標のリセットが必要な時用
		void ResetMousePos(glm::vec2 mousePos) { m_lastMousePos = mousePos; }

		void Update(const UpdateData& updateData) override;
		//void BindDebugGUIElement() override;

		virtual rapidjson::Value Serialize(rapidjson::Document::AllocatorType& allocator) const { return __super::Serialize(allocator); }
		virtual void Deserialize(const rapidjson::Value& doc, std::function<void(ObjectType, const rapidjson::Value&)> createObject) { __super::Deserialize(doc, createObject); }

	private:
		virtual void initialize() override;
		glm::vec2 m_lastMousePos;

	};
	TYPEDEF_SHARED_PTR(FPSCamera)
		OBJECT_CAST_FUNCTION(FPSCamera)
}