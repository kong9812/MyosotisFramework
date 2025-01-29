#pragma once
#include "camera.h"

namespace MyosotisFW::System::Render::Camera
{
	class EditorCamera : public CameraBase
	{
	public:
		EditorCamera();
		~EditorCamera() {};

		glm::mat4 GetViewMatrix() const override;
		glm::mat4 GetProjectionMatrix() const override;
		glm::vec3 GetWorldPos(glm::vec2 pos, float distance) const override;

		virtual const ObjectType GetObjectType() const override { return ObjectType::FPSCamera; }

		// 一時停止などマウス座標のリセットが必要な時用
		void ResetMousePos(glm::vec2 mousePos) { m_lastMousePos = mousePos; }

		void Update(const Utility::Vulkan::Struct::UpdateData& updateData) override;
		void BindDebugGUIElement() override;

		virtual rapidjson::Value Serialize(rapidjson::Document::AllocatorType& allocator) const { return __super::Serialize(allocator); }
		virtual void Deserialize(const rapidjson::Value& doc, std::function<void(ObjectType, const rapidjson::Value&)> createObject) { __super::Deserialize(doc, createObject); }

	private:
		glm::vec2 m_lastMousePos;
	};
	TYPEDEF_SHARED_PTR(EditorCamera)
	OBJECT_CAST_FUNCTION(EditorCamera)
}