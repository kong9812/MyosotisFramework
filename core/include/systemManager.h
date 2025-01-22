// Copyright (c) 2025 kong9812
#pragma once
#include "classPointer.h"

#include "libs/glfw.h"
#include "subsystem/renderSubsystem.h"

namespace MyosotisFW::System
{
	class SystemManager
	{
	public:
		SystemManager(GLFWwindow* window);
		~SystemManager();

		Render::RenderSubsystem_ptr& GetRenderSubsystem() { return m_renderSubsystem; }
		VkSurfaceKHR& GetSurface() { return m_surface; }

		void KeyAction(int key, int action);
		void CursorMotion(glm::vec2 pos);
		void Pause(GLFWwindow* window);

		void Update();
		void Render();

	private:
		Render::RenderSubsystem_ptr m_renderSubsystem;
		
		VkInstance m_instance;
		VkSurfaceKHR m_surface;

		bool m_pause;
		float m_lastTime;

		// 入力
		std::unordered_map<int, int> m_keyActions;
		glm::vec2 m_mousePos;

		// debug callback
		VkDebugReportCallbackEXT m_vkDebugReportCallback;
		PFN_vkCreateDebugReportCallbackEXT m_vkCreateDebugReportCallbackEXT;
		PFN_vkDestroyDebugReportCallbackEXT m_vkDestroyDebugReportCallbackEXT;

		// glfw callback
		static void ResizedCallback(GLFWwindow* window, int width, int height);
		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
	};
	TYPEDEF_SHARED_PTR_ARGS(SystemManager)
}
