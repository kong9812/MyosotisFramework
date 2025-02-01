// Copyright (c) 2025 kong9812
#pragma once
#include "classPointer.h"

#include "iglfw.h"
#include "RenderSubsystem.h"
#include "GameDirector.h"

namespace MyosotisFW::System
{
	class SystemManager
	{
	public:
		SystemManager(GLFWwindow* window);
		~SystemManager();

		Render::RenderSubsystem_ptr& GetRenderSubsystem() { return m_renderSubsystem; }
		GameDirector::GameDirector_ptr& GetGameDirector() { return m_gameDirector; }

		VkSurfaceKHR& GetSurface() { return m_surface; }

		void KeyAction(int key, int action);
		void MouseButtonAction(int key, int action);
		void CursorMotion(glm::vec2 pos);
		void Pause(GLFWwindow* window);

		void Update();
		void Render();

	private:
		Render::RenderSubsystem_ptr m_renderSubsystem;
		GameDirector::GameDirector_ptr m_gameDirector;
		
		VkInstance m_instance;
		VkSurfaceKHR m_surface;

		bool m_pause;
		float m_lastTime;

		// 入力
		std::unordered_map<int, int> m_keyActions;
		std::unordered_map<int, int> m_mouseButtonActions;
		glm::vec2 m_mousePos;

		// debug callback
		VkDebugReportCallbackEXT m_vkDebugReportCallback;
		PFN_vkCreateDebugReportCallbackEXT m_vkCreateDebugReportCallbackEXT;
		PFN_vkDestroyDebugReportCallbackEXT m_vkDestroyDebugReportCallbackEXT;

		// glfw callback
		static void ResizedCallback(GLFWwindow* window, int width, int height);
		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
		static void DropCallback(GLFWwindow* window, int path_count, const char* paths[]);
	};
	TYPEDEF_SHARED_PTR_ARGS(SystemManager)
}
