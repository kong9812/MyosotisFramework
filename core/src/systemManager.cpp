// Copyright (c) 2025 kong9812
#include "systemManager.h"

#include <vector>

#include "vkCreateInfo.h"
#include "vkValidation.h"
#include "appInfo.h"
#include "logger.h"

namespace {
	// デバッグコールバック関数
	VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objectType,
		uint64_t object,
		size_t location,
		int32_t messageCode,
		const char* pLayerPrefix,
		const char* pMessage,
		void* pUserData)
	{
		std::string message = std::string("[") + pLayerPrefix + "] " + pMessage;

		switch (flags)
		{
		case VkDebugReportFlagBitsEXT::VK_DEBUG_REPORT_INFORMATION_BIT_EXT:
		{
			Logger::Info(message);
		}
		break;
		case VkDebugReportFlagBitsEXT::VK_DEBUG_REPORT_DEBUG_BIT_EXT:
		{
			Logger::Debug(message);
		}
		break;
		case VkDebugReportFlagBitsEXT::VK_DEBUG_REPORT_WARNING_BIT_EXT:
		{
			Logger::Warning(message);
		}
		break;
		case VkDebugReportFlagBitsEXT::VK_DEBUG_REPORT_ERROR_BIT_EXT:
		default:
		{
			ASSERT(false, message);
		}
		break;
		}
		return VK_FALSE;  // 通常通り処理を続行
	}
}

namespace MyosotisFW::System
{
	SystemManager::SystemManager(GLFWwindow* window)
	{
		// applicationInfo
		VkApplicationInfo applicationInfo = Utility::Vulkan::CreateInfo::applicationInfo(
			AppInfo::g_applicationName,
			AppInfo::g_engineName,
			AppInfo::g_apiVersion,
			AppInfo::g_engineVersion);

		// extension
		std::vector<VkExtensionProperties> extensionProperties{};
		{
			uint32_t count = 0;
			vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
			extensionProperties.resize(count);
			vkEnumerateInstanceExtensionProperties(nullptr, &count, extensionProperties.data());
#ifdef DEBUG
			for (const auto& extensionPropertie : extensionProperties)
			{
				Logger::Info(std::string("extensionName: ") + extensionPropertie.extensionName + "(" + std::to_string(extensionPropertie.specVersion) + ")");
			}
#endif
		}

		// vulkan instance
		VkInstanceCreateInfo instanceCreateInfo = Utility::Vulkan::CreateInfo::instanceCreateInfo(applicationInfo, AppInfo::g_vkInstanceExtensionProperties, AppInfo::g_layer);		
		VK_VALIDATION(vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance));

		// prepare for VK_EXT_DEBUG_REPORT_EXTENSION
		m_vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(m_instance, "vkCreateDebugReportCallbackEXT"));
		m_vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(m_instance, "vkDestroyDebugReportCallbackEXT"));
		VkDebugReportCallbackCreateInfoEXT vebugReportCallbackCreateInfo{};
		vebugReportCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		vebugReportCallbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT;
		vebugReportCallbackCreateInfo.pfnCallback = &debugCallback;
		m_vkCreateDebugReportCallbackEXT(m_instance, &vebugReportCallbackCreateInfo, nullptr, &m_vkDebugReportCallback);

		// GLFW サーフェース作成
		VK_VALIDATION(glfwCreateWindowSurface(m_instance, window, nullptr, &m_surface));

		// m_renderSubsystem
		m_renderSubsystem = Render::CreateRenderSubsystemPointer(*window, m_instance, m_surface);

		m_pause = false;

		// リサイズコールバック
		glfwSetWindowUserPointer(window, this);
		glfwSetWindowSizeCallback(window, ResizedCallback);
		// キー & マウスコールバック
		glfwSetKeyCallback(window, KeyCallback);
		glfwSetCursorPosCallback(window, CursorPosCallback);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		double x{}, y{};
		glfwGetCursorPos(window, &x, &y);
		m_mousePos = glm::vec2(static_cast<float>(x), static_cast<float>(y));
	}

	SystemManager::~SystemManager()
	{
		m_renderSubsystem.reset();

		if (m_vkDestroyDebugReportCallbackEXT)
		{
			m_vkDestroyDebugReportCallbackEXT(m_instance, m_vkDebugReportCallback, nullptr);
		}
		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
		vkDestroyInstance(m_instance, nullptr);
	}

	void SystemManager::KeyAction(int key, int action)
	{
		m_keyActions.insert_or_assign(key, action);
	}

	void SystemManager::CursorMotion(glm::vec2 pos)
	{
		m_mousePos = pos;
	}

	void SystemManager::Pause(GLFWwindow* window)
	{
		if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
		{
			m_pause = true;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		else
		{
			m_pause = false;
			double x{}, y{};
			glfwGetCursorPos(window, &x, &y);
			m_mousePos = glm::vec2(static_cast<float>(x), static_cast<float>(y));
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}	
	}

	void SystemManager::Update()
	{
		float currentTime = static_cast<float>(glfwGetTime());
		float deltaTime = currentTime - m_lastTime;
		m_renderSubsystem->Update({ m_pause, deltaTime, m_keyActions, m_mousePos });

		// 後片付け
		for (auto it = m_keyActions.begin(); it != m_keyActions.end();)
		{
			if (it->second == GLFW_RELEASE)
			{
				it = m_keyActions.erase(it);
			}
			else
			{
				it++;
			}
		}
		m_lastTime = currentTime;
	}

	void SystemManager::Render()
	{
		m_renderSubsystem->Render();
	}

	void SystemManager::ResizedCallback(GLFWwindow* window, int width, int height)
	{
		SystemManager* systemManager = static_cast<SystemManager*>(glfwGetWindowUserPointer(window));
		ASSERT(systemManager != nullptr, "Could not find WindowUserPointer!");
		systemManager->GetRenderSubsystem()->Resize(systemManager->GetSurface(), width, height);
	}

	void SystemManager::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		SystemManager* systemManager = static_cast<SystemManager*>(glfwGetWindowUserPointer(window));
		ASSERT(systemManager != nullptr, "Could not find WindowUserPointer!");
		systemManager->KeyAction(key, action);
		if ((key == GLFW_KEY_F2) && (action == GLFW_PRESS))
		{
			systemManager->Pause(window);
		}
	}

	void SystemManager::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
	{
		SystemManager* systemManager = static_cast<SystemManager*>(glfwGetWindowUserPointer(window));
		ASSERT(systemManager != nullptr, "Could not find WindowUserPointer!");
		systemManager->CursorMotion(glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos)));
	}
}