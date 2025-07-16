// Copyright (c) 2025 kong9812
#include "RenderDevice.h"

#include "RenderQueue.h"

#include "VK_CreateInfo.h"
#include "AppInfo.h"
#include "Logger.h"

namespace
{
#ifdef DEBUG
	void PrintPhysicalDeviceInfo(VkPhysicalDevice physicalDevice) {
		// デバイスプロパティを取得
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);

		// デバイス情報をログに出力
		CustomLog(LogLevel::LOG_INFO, "=============================================================================");
		CustomLog(LogLevel::LOG_INFO, "Device Name: " + std::string(properties.deviceName));
		CustomLog(LogLevel::LOG_INFO, "API Version: " +
			std::to_string(VK_VERSION_MAJOR(properties.apiVersion)) + "." +
			std::to_string(VK_VERSION_MINOR(properties.apiVersion)) + "." +
			std::to_string(VK_VERSION_PATCH(properties.apiVersion)));
		CustomLog(LogLevel::LOG_INFO, "Driver Version: " + std::to_string(properties.driverVersion));
		CustomLog(LogLevel::LOG_INFO, "Vendor ID: " + std::to_string(properties.vendorID));
		CustomLog(LogLevel::LOG_INFO, "Device ID: " + std::to_string(properties.deviceID));
		std::string deviceType;
		switch (properties.deviceType) {
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			deviceType = "Discrete GPU";
			break;
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			deviceType = "Integrated GPU";
			break;
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			deviceType = "Virtual GPU";
			break;
		case VK_PHYSICAL_DEVICE_TYPE_CPU:
			deviceType = "CPU";
			break;
		default:
			deviceType = "Other";
			break;
		}
		CustomLog(LogLevel::LOG_INFO, "Device Type: " + deviceType);
		CustomLog(LogLevel::LOG_INFO, "=============================================================================");
	}
#endif
	void* vkAllocation(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
	{
		void* ptr = malloc(size);
		std::string message = "vkAllocation ";
		message += std::string("address: ") + std::to_string(reinterpret_cast<uintptr_t>(ptr)) + " ";
		message += "size: " + std::to_string(size) + " ";
		message += "alignment: " + std::to_string(alignment) + " ";
		message += std::string("allocationScope: ") + vmaTools::VkSystemAllocationScopeToString(allocationScope).data();
		Logger::Info(message);
		return ptr;
	}
	void* vkReallocation(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
	{
		void* ptr = std::realloc(pOriginal, size);
		std::string message = "vkReallocation ";
		message += std::string("address: ") + std::to_string(reinterpret_cast<uintptr_t>(ptr)) + " ";
		message += "size: " + std::to_string(size) + " ";
		message += "alignment: " + std::to_string(alignment) + " ";
		message += std::string("allocationScope: ") + vmaTools::VkSystemAllocationScopeToString(allocationScope).data();
		Logger::Info(message);
		return ptr;
	}
	void vkFree(void* pUserData, void* pMemory)
	{
		if (!pMemory) return;
		std::string message = "vkFree ";
		message += std::string("address: ") + std::to_string(reinterpret_cast<uintptr_t>(pMemory)) + " ";
		Logger::Info(message);
		free(pMemory);
		pMemory = nullptr;
	}
	void vkInternalAllocationNotification(void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope)
	{
		std::string message = "vkInternalAllocationNotification ";
		message += "size: " + std::to_string(size) + " ";
		message += std::string("allocationType: ") + vmaTools::VkInternalAllocationTypeToString(allocationType).data() + " ";
		message += std::string("allocationScope: ") + vmaTools::VkSystemAllocationScopeToString(allocationScope).data();
		Logger::Info(message);
	}
	void vkInternalFreeNotification(void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope)
	{
		std::string message = "vkInternalFreeNotification ";
		message += "size: " + std::to_string(size) + " ";
		message += std::string("allocationType: ") + vmaTools::VkInternalAllocationTypeToString(allocationType).data() + " ";
		message += std::string("allocationScope: ") + vmaTools::VkSystemAllocationScopeToString(allocationScope).data();
		Logger::Info(message);
	}
}

namespace MyosotisFW::System::Render
{
	RenderDevice::RenderDevice(const VkInstance& vkInstance)
	{
		// prepare allocation callbacks
		prepareAllocationCallbacks();

		// physical device
		uint32_t physicalDeviceCount = 0;
		VK_VALIDATION(vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, nullptr));
		std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
		VK_VALIDATION(vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, physicalDevices.data()));
		m_physicalDevice = physicalDevices[AppInfo::g_physicalIndex];
#ifdef DEBUG
		PrintPhysicalDeviceInfo(m_physicalDevice);
#endif

		// memory properties
		vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_physicalDeviceMemoryProperties);

		// queue family properties
		std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos{};
		const float defaultQueuePriority(0.0f);
		{// Graphics queue
			m_graphicsQueue = CreateRenderQueuePointer(m_physicalDevice, VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT);
			deviceQueueCreateInfos.push_back(Utility::Vulkan::CreateInfo::deviceQueueCreateInfo(m_graphicsQueue->GetQueueFamilyIndex(), defaultQueuePriority));
		}
		{// Compute queue
			m_computeQueue = CreateRenderQueuePointer(m_physicalDevice, VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT);
			deviceQueueCreateInfos.push_back(Utility::Vulkan::CreateInfo::deviceQueueCreateInfo(m_computeQueue->GetQueueFamilyIndex(), defaultQueuePriority));
		}
		{// Transfer queue
			m_transferQueue = CreateRenderQueuePointer(m_physicalDevice, VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT);
			deviceQueueCreateInfos.push_back(Utility::Vulkan::CreateInfo::deviceQueueCreateInfo(m_transferQueue->GetQueueFamilyIndex(), defaultQueuePriority));
		}

		// extensions
		uint32_t extCount = 0;
		VK_VALIDATION(vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extCount, nullptr));
		std::vector<VkExtensionProperties> extensionProperties(extCount);
		VK_VALIDATION(vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extCount, extensionProperties.data()));
#ifdef DEBUG
		for (VkExtensionProperties& extensionPropertie : extensionProperties)
		{
			Logger::Info(std::string("extensionName: ") + extensionPropertie.extensionName + "(" + std::to_string(extensionPropertie.specVersion) + ")");
		}
#endif
		// デバイス機能を取得
		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceFeatures(m_physicalDevice, &features);

		// create device
		VkDeviceCreateInfo deviceCreateInfo = Utility::Vulkan::CreateInfo::deviceCreateInfo(deviceQueueCreateInfos, AppInfo::g_vkDeviceExtensionProperties, features);
		VK_VALIDATION(vkCreateDevice(m_physicalDevice, &deviceCreateInfo, GetAllocationCallbacks(), &m_device));

		// queue instance
		m_graphicsQueue->CreateQueueInstance(m_device);
		m_computeQueue->CreateQueueInstance(m_device);
		m_transferQueue->CreateQueueInstance(m_device);

		// VMA
		prepareVMA(vkInstance);
	}

	RenderDevice::~RenderDevice()
	{
		vmaDestroyAllocator(m_allocator);
		vkDestroyDevice(m_device, GetAllocationCallbacks());
	}

	void RenderDevice::ImageMemoryAllocate(DeviceImage& deviceImage)
	{
		VkMemoryRequirements memReqs{};
		vkGetImageMemoryRequirements(m_device, deviceImage.image, &memReqs);
		VkMemoryAllocateInfo memoryAllocateInfo = Utility::Vulkan::CreateInfo::memoryAllocateInfo(memReqs.size, getMemoryTypeIndex(memReqs.memoryTypeBits, VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
		VK_VALIDATION(vkAllocateMemory(m_device, &memoryAllocateInfo, GetAllocationCallbacks(), &deviceImage.memory));
		VK_VALIDATION(vkBindImageMemory(m_device, deviceImage.image, deviceImage.memory, 0));
	}

	uint32_t RenderDevice::getMemoryTypeIndex(const uint32_t& typeBits, const VkMemoryPropertyFlags& properties) const
	{
		for (uint32_t i = 0; i < m_physicalDeviceMemoryProperties.memoryTypeCount; i++) {
			// タイプフィルタが一致し、プロパティ要件を満たしているか確認
			if ((typeBits & (1 << i)) &&
				(m_physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		ASSERT(false, "Could not find a matching memory type index!");
		return 0;
	}

	void RenderDevice::prepareAllocationCallbacks()
	{
		m_allocationCallbacks.pfnAllocation = &vkAllocation;
		m_allocationCallbacks.pfnFree = &vkFree;
		m_allocationCallbacks.pfnReallocation = &vkReallocation;
		m_allocationCallbacks.pfnInternalAllocation = &vkInternalAllocationNotification;
		m_allocationCallbacks.pfnInternalFree = &vkInternalFreeNotification;
	}

	void RenderDevice::prepareVMA(const VkInstance& vkInstance)
	{
		VmaVulkanFunctions vulkanFunctions{};
		vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
		vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

		VmaAllocatorCreateInfo allocatorCreateInfo{};
		allocatorCreateInfo.physicalDevice = m_physicalDevice;
		allocatorCreateInfo.device = m_device;
		allocatorCreateInfo.instance = vkInstance;
		allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;
		allocatorCreateInfo.pAllocationCallbacks = &m_allocationCallbacks;
		VK_VALIDATION(vmaCreateAllocator(&allocatorCreateInfo, &m_allocator));
	}
}