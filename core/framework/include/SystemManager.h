// Copyright (c) 2025 kong9812
#pragma once
#include "ClassPointer.h"

#include "iglfw.h"
#include "RenderSubsystem.h"
#include "GameDirector.h"

namespace MyosotisFW::System
{
	class SystemManager
	{
	public:
		SystemManager() :
			m_renderSubsystem(nullptr),
			m_gameDirector(nullptr),
			m_instance(VK_NULL_HANDLE),
			m_surface(VK_NULL_HANDLE),
			m_pause(false),
			m_lastTime(0.0f),
			m_vkDebugReportCallback(VK_NULL_HANDLE),
			m_vkCreateDebugReportCallbackEXT(nullptr),
			m_vkDestroyDebugReportCallbackEXT(nullptr),
			m_mousePos(glm::vec2(0.0f)) {
		};
		~SystemManager();

		Render::RenderSubsystem_ptr& GetRenderSubsystem() { return m_renderSubsystem; }
		GameDirector::GameDirector_ptr& GetGameDirector() { return m_gameDirector; }

		VkSurfaceKHR& GetSurface() { return m_surface; }

		void KeyAction(const int& key, const int& action);
		void MouseButtonAction(const int& key, const int& action);
		void CursorMotion(glm::vec2 pos);
		void Pause(GLFWwindow* window);

		virtual void Initialize(GLFWwindow* window);
		virtual void Update();
		virtual void Render();

	protected:
		void initializeGLFW(GLFWwindow* window);
		void initializeVulkanApplication(GLFWwindow* window);
		virtual void initializeRenderSubsystem(GLFWwindow* window);
		virtual void initializeGameDirector();

	protected:
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
		static void resizedCallback(GLFWwindow* window, int width, int height);
		static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
		static void dropCallback(GLFWwindow* window, int path_count, const char* paths[]);
	};
	TYPEDEF_SHARED_PTR_ARGS(SystemManager)
}
