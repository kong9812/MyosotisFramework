// Copyright (c) 2025 kong9812
#pragma once
#include "Camera.h"

namespace MyosotisFW::System::Render::Camera
{
	class EditorCamera : public CameraBase
	{
	public:
		EditorCamera();
		~EditorCamera() {};

		glm::mat4 GetViewMatrix() const override;
		glm::mat4 GetProjectionMatrix() const override;
		glm::vec3 GetWorldPos(const glm::vec2& pos, const float& distance) const override;

		virtual const ComponentType GetType() const override { return ComponentType::FPSCamera; }

		// 一時停止などマウス座標のリセットが必要な時用
		void ResetMousePos(glm::vec2 mousePos) { m_lastMousePos = mousePos; }

		void Update(const UpdateData& updateData) override;

	private:
		virtual void initialize() override;
		glm::vec2 m_lastMousePos;

		void editorGUI();
	};
	TYPEDEF_SHARED_PTR(EditorCamera);
	OBJECT_CAST_FUNCTION(EditorCamera);
}