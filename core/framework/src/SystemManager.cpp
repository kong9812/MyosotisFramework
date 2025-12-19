// Copyright (c) 2025 kong9812
#include "SystemManager.h"

#include <vector>

#include "GameDirector.h"
#include "RenderSubsystem.h"

#include "VK_CreateInfo.h"
#include "VK_Validation.h"
#include "AppInfo.h"
#include "Logger.h"
#include "Structs.h"

#include "ComponentFactory.h"

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
	SystemManager::~SystemManager()
	{
		m_gameDirector.reset();
		m_renderSubsystem.reset();

		if (m_vkDestroyDebugReportCallbackEXT)
		{
			m_vkDestroyDebugReportCallbackEXT(m_instance, m_vkDebugReportCallback, nullptr);
		}
		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
		vkDestroyInstance(m_instance, nullptr);
	}

	void SystemManager::KeyAction(const int key, const int action)
	{
		m_keyActions.insert_or_assign(key, action);
	}

	void SystemManager::MouseButtonAction(const int key, const int action)
	{
		m_mouseButtonActions.insert_or_assign(key, action);
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
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfwPollEvents();
			double x{}, y{};
			glfwGetCursorPos(window, &x, &y);
			m_mousePos = glm::vec2(static_cast<float>(x), static_cast<float>(y));
			m_renderSubsystem->ResetMousePos(m_mousePos);
		}
	}

	void SystemManager::Initialize(GLFWwindow* window)
	{
		// GLFW初期化
		initializeGLFW(window);

		// Vulkan初期化
		initializeVulkanApplication(window);

		// RenderSubsystem初期化
		initializeRenderSubsystem(window);

		// GameDirector初期化
		initializeGameDirector();

		// マウス初期化
		double x{}, y{};
		glfwGetCursorPos(window, &x, &y);
		m_mousePos = glm::vec2(static_cast<float>(x), static_cast<float>(y));
	}

	void SystemManager::Update()
	{
		float currentTime = static_cast<float>(glfwGetTime());
		float deltaTime = currentTime - m_lastTime;
		m_renderSubsystem->Update({ m_pause, deltaTime, m_keyActions, m_mouseButtonActions, m_mousePos });

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
		for (auto it = m_mouseButtonActions.begin(); it != m_mouseButtonActions.end();)
		{
			if (it->second == GLFW_RELEASE)
			{
				it = m_mouseButtonActions.erase(it);
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
		m_renderSubsystem->BeginRender();
		m_renderSubsystem->BeginCompute();
		m_renderSubsystem->SkyboxRender();
		m_renderSubsystem->MeshShaderRender();
		m_renderSubsystem->LightingRender();
		m_renderSubsystem->LightmapBake();
		m_renderSubsystem->RayTracingRender();
		m_renderSubsystem->EndRender();
	}

	void SystemManager::initializeGLFW(GLFWwindow* window)
	{
		// リサイズコールバック
		glfwSetWindowUserPointer(window, this);
		glfwSetWindowSizeCallback(window, resizedCallback);
		// D&D
		glfwSetDropCallback(window, dropCallback);
		// キー & マウスコールバック
		glfwSetKeyCallback(window, keyCallback);
		glfwSetMouseButtonCallback(window, mouseButtonCallback);
		glfwSetCursorPosCallback(window, cursorPosCallback);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwPollEvents();
	}

	void SystemManager::initializeVulkanApplication(GLFWwindow* window)
	{
		// applicationInfo
		VkApplicationInfo applicationInfo = Utility::Vulkan::CreateInfo::applicationInfo(AppInfo::g_applicationName, AppInfo::g_engineName, AppInfo::g_apiVersion, AppInfo::g_engineVersion);

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
	}

	void SystemManager::initializeRenderSubsystem(GLFWwindow* window)
	{
		m_renderSubsystem = Render::CreateRenderSubsystemPointer();
		m_renderSubsystem->Initialize(m_instance, m_surface);
	}

	void SystemManager::initializeGameDirector()
	{
		// m_gameDirector
		m_gameDirector = GameDirector::CreateGameDirectorPointer(m_renderSubsystem);
	}

	void SystemManager::resizedCallback(GLFWwindow* window, int width, int height)
	{
		SystemManager* systemManager = static_cast<SystemManager*>(glfwGetWindowUserPointer(window));
		ASSERT(systemManager != nullptr, "Could not find WindowUserPointer!");
		systemManager->GetRenderSubsystem()->Resize(systemManager->GetSurface(), width, height);
	}

	void SystemManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		SystemManager* systemManager = static_cast<SystemManager*>(glfwGetWindowUserPointer(window));
		ASSERT(systemManager != nullptr, "Could not find WindowUserPointer!");
		systemManager->KeyAction(key, action);
	}

	void SystemManager::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
	{
		SystemManager* systemManager = static_cast<SystemManager*>(glfwGetWindowUserPointer(window));
		ASSERT(systemManager != nullptr, "Could not find WindowUserPointer!");
		systemManager->MouseButtonAction(button, action);
	}

	void SystemManager::cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
	{
		SystemManager* systemManager = static_cast<SystemManager*>(glfwGetWindowUserPointer(window));
		ASSERT(systemManager != nullptr, "Could not find WindowUserPointer!");
		systemManager->CursorMotion(glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos)));
	}

	void SystemManager::dropCallback(GLFWwindow* window, int path_count, const char* paths[])
	{
		if (path_count > 1)
		{
			Logger::Error("Cant dorp more than one file.");
			return;
		}
		SystemManager* systemManager = static_cast<SystemManager*>(glfwGetWindowUserPointer(window));
		ASSERT(systemManager != nullptr, "Could not find WindowUserPointer!");
		Render::RenderSubsystem_ptr renderSubsystem = systemManager->GetRenderSubsystem();
	}
}