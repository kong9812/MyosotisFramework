// Copyright (c) 2025 kong9812
#pragma once
#include "camera.h"
#include "classPointer.h"

namespace MyosotisFW::System::Render::Camera
{
	class FPSCamera : public CameraBase
	{
	public:
		FPSCamera() : CameraBase(){};
		~FPSCamera() {};

		glm::mat4 GetViewMatrix() const override;
		glm::mat4 GetProjectionMatrix() const override;

		void ProcessMouseMovement(float xOffset, float yOffset) override {};
		void ProcessMouseZoom(float offset) override {};
		void ProcessKeyboardInput(const std::string& direction, float deltaTime) override {};
	private:
	};
	TYPEDEF_SHARED_PTR(FPSCamera)
}