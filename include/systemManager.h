#pragma once
#include <memory>

#include "libs/glfw.h"
#include "subsystem/renderSubsystem.h"

namespace MyosotisFW::System
{
	class SystemManager
	{
	public:
		SystemManager(GLFWwindow* window);
		~SystemManager();

		Render::RenderSubsystem_prt& GetRenderSubsystem() { return m_renderSubsystem; }
		VkSurfaceKHR& GetSurface() { return m_surface; }

		void Update();

	private:
		Render::RenderSubsystem_prt m_renderSubsystem;

		VkInstance m_instance;
		VkSurfaceKHR m_surface;

		VkDebugReportCallbackEXT m_vkDebugReportCallback;
		PFN_vkCreateDebugReportCallbackEXT m_vkCreateDebugReportCallbackEXT;
		PFN_vkDestroyDebugReportCallbackEXT m_vkDestroyDebugReportCallbackEXT;

		static void ResizedCallback(GLFWwindow* window, int width, int height);
	};
}
