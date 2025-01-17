// Copyright (c) 2025 kong9812
#include "subsystem/render/renderDevice.h"

#include "vkCreateInfo.h"
#include "appInfo.h"
#include "logger.h"

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
}

namespace MyosotisFW::System::Render
{
	RenderDevice::RenderDevice(VkInstance& vkInstance)
	{
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
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
        m_queueFamilyProperties.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, m_queueFamilyProperties.data());
        std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos{};
        const float defaultQueuePriority(0.0f);
        {// Graphics queue
            m_graphicsFamilyIndex = getQueueFamilyIndex(VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT, m_queueFamilyProperties);
            deviceQueueCreateInfos.push_back(Utility::Vulkan::CreateInfo::deviceQueueCreateInfo(m_graphicsFamilyIndex, defaultQueuePriority));
        }
        {// Compute queue
            m_computeFamilyIndex = getQueueFamilyIndex(VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT, m_queueFamilyProperties);
            deviceQueueCreateInfos.push_back(Utility::Vulkan::CreateInfo::deviceQueueCreateInfo(m_computeFamilyIndex, defaultQueuePriority));
        }
        {// Transfer queue
            m_transferFamilyIndex = getQueueFamilyIndex(VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT, m_queueFamilyProperties);
            deviceQueueCreateInfos.push_back(Utility::Vulkan::CreateInfo::deviceQueueCreateInfo(m_transferFamilyIndex, defaultQueuePriority));
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
        VK_VALIDATION(vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device));
	}

	RenderDevice::~RenderDevice()
	{
        if (m_device)
        {
            vkDestroyDevice(m_device, nullptr);
        }
    }

    void RenderDevice::ImageMemoryAllocate(Utility::Vulkan::Struct::DeviceImage& deviceImage)
    {
        VkMemoryRequirements memReqs{};
        vkGetImageMemoryRequirements(m_device, deviceImage.image, &memReqs);
        VkMemoryAllocateInfo memoryAllocateInfo = Utility::Vulkan::CreateInfo::memoryAllocateInfo(memReqs.size, getMemoryTypeIndex(memReqs.memoryTypeBits, VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
        VK_VALIDATION(vkAllocateMemory(m_device, &memoryAllocateInfo, nullptr, &deviceImage.memory));
        VK_VALIDATION(vkBindImageMemory(m_device, deviceImage.image, deviceImage.memory, 0));
    }

    void RenderDevice::CreateBuffer(Utility::Vulkan::Struct::Buffer& buffer, uint32_t bufferSize, VkBufferUsageFlagBits usage, VkMemoryPropertyFlags memoryPropertyFlags)
    {
        // Buffer
        VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(bufferSize, usage);
        VK_VALIDATION(vkCreateBuffer(m_device, &bufferCreateInfo, nullptr, &buffer.buffer));

        // Memory allocate
        VkMemoryRequirements memoryRequirements{};
        vkGetBufferMemoryRequirements(m_device, buffer.buffer, &memoryRequirements);
        VkMemoryAllocateInfo memoryAllocateInfo = Utility::Vulkan::CreateInfo::memoryAllocateInfo(memoryRequirements.size, getMemoryTypeIndex(memoryRequirements.memoryTypeBits, memoryPropertyFlags));
        VK_VALIDATION(vkAllocateMemory(m_device, &memoryAllocateInfo, nullptr, &buffer.memory));

        // Descriptor buffer info
        buffer.descriptor.buffer = buffer.buffer;   // UBOバッファ
        buffer.descriptor.offset = 0;               // バッファの開始位置（通常は0）
        buffer.descriptor.range = bufferSize;		// UBOデータのサイズ

        // Bind
        VK_VALIDATION(vkBindBufferMemory(m_device, buffer.buffer, buffer.memory, buffer.descriptor.offset));

        // Map
        VK_VALIDATION(vkMapMemory(m_device, buffer.memory, buffer.descriptor.offset, bufferSize, 0, &buffer.mapped));
    }

    uint32_t RenderDevice::getQueueFamilyIndex(VkQueueFlags queueFlags, const std::vector<VkQueueFamilyProperties>& queueFamilyProperties)
    {
        // VK_QUEUE_COMPUTE_BIT Only
        if ((queueFlags & VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT) == queueFlags)
        {
            for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
            {
                if ((queueFamilyProperties[i].queueFlags & VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT) && ((queueFamilyProperties[i].queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT) == 0))
                {
                    return i;
                }
            }
        }

        // VK_QUEUE_TRANSFER_BIT Only
        if ((queueFlags & VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT) == queueFlags)
        {
            for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
            {
                if ((queueFamilyProperties[i].queueFlags & VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT) && ((queueFamilyProperties[i].queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT) == 0) && ((queueFamilyProperties[i].queueFlags & VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT) == 0))
                {
                    return i;
                }
            }
        }

        // Any
        for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
        {
            if ((queueFamilyProperties[i].queueFlags & queueFlags) == queueFlags)
            {
                return i;
            }
        }

        ASSERT(false, "Could not find a matching queue!");
        return 0;
    }

    uint32_t RenderDevice::getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties)
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
}