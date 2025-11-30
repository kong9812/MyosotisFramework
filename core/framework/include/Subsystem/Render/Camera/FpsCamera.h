// Copyright (c) 2025 kong9812
#pragma once
#include "Camera.h"
#include "ClassPointer.h"
#include "ComponentCast.h"

namespace MyosotisFW::System::Render::Camera
{
	class FPSCamera : public CameraBase
	{
	public:
		FPSCamera(const uint32_t objectID);
		~FPSCamera() {};

		glm::mat4 GetViewMatrix() const override;
		glm::mat4 GetProjectionMatrix() const override;
		glm::vec3 GetWorldPos(const glm::vec2& pos, const float distance) const override;

		virtual const ComponentType GetType() const override { return ComponentType::FPSCamera; }

		// 一時停止などマウス座標のリセットが必要な時用
		void ResetMousePos(const glm::vec2& mousePos) { m_lastMousePos = mousePos; }

		void Update(const UpdateData& updateData) override;

	private:
		virtual void initialize() override;
		glm::vec2 m_lastMousePos;

	};
	TYPEDEF_SHARED_PTR_ARGS(FPSCamera);
	OBJECT_CAST_FUNCTION(FPSCamera);
}