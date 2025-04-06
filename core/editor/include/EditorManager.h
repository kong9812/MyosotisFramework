// Copyright (c) 2025 kong9812
#pragma once
#include "SystemManager.h"

namespace MyosotisFW::System
{
	class EditorManager :public SystemManager
	{
	public:
		EditorManager() : SystemManager() {};
		~EditorManager() {};

		void Initialize(GLFWwindow* window) override;
		void Update() override;
		void Render() override;

	private:
		void initializeRenderSubsystem(GLFWwindow* window) override;
		void initializeGameDirector();

	};
	TYPEDEF_SHARED_PTR_ARGS(EditorManager)
}
