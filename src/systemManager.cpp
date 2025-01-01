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
		m_renderSubsystem = std::make_unique<Render::RenderSubsystem>(m_instance, m_surface);

		// リサイズコールバック
		glfwSetWindowUserPointer(window, this);
		glfwSetWindowSizeCallback(window, ResizedCallback);
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

	void SystemManager::Update()
	{
		m_renderSubsystem->Render();
	}

	void SystemManager::ResizedCallback(GLFWwindow* window, int width, int height)
	{
		SystemManager* systemManager = static_cast<SystemManager*>(glfwGetWindowUserPointer(window));
		ASSERT(systemManager != nullptr, "Could not find WindowUserPointer!");
		systemManager->GetRenderSubsystem()->Resize(systemManager->GetSurface(), width, height);
	}
}