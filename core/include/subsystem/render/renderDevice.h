// Copyright (c) 2025 kong9812
#pragma once
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>
#include "classPointer.h"
#include "vkStruct.h"
#include "vkValidation.h"

namespace MyosotisFW::System::Render
{
	class RenderDevice
	{
	public:
		RenderDevice(VkInstance& vkInstance);
		~RenderDevice();

		operator VkDevice() const
		{
			return m_device;
		};

		VkPhysicalDevice GetPhysicalDevice() { return m_physicalDevice; }
		uint32_t GetGraphicsFamilyIndex() { return m_graphicsFamilyIndex; }
		uint32_t GetComputeFamilyIndex() { return m_computeFamilyIndex; }
		uint32_t GetTransferFamilyIndex() { return m_transferFamilyIndex; }

		void ImageMemoryAllocate(Utility::Vulkan::Struct::DeviceImage& deviceImage);

		void CreateBuffer(Utility::Vulkan::Struct::Buffer& buffer, uint32_t bufferSize, VkBufferUsageFlagBits usage, VkMemoryPropertyFlags memoryPropertyFlags);

	private:
		VkPhysicalDevice m_physicalDevice;
		VkDevice m_device;

		uint32_t m_graphicsFamilyIndex;
		uint32_t m_computeFamilyIndex;
		uint32_t m_transferFamilyIndex;

		std::vector<VkQueueFamilyProperties> m_queueFamilyProperties;
		VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProperties;
	private:
		uint32_t getQueueFamilyIndex(VkQueueFlags queueFlags, const std::vector<VkQueueFamilyProperties>& queueFamilyProperties);
		uint32_t getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties);
	};
	TYPEDEF_SHARED_PTR_ARGS(RenderDevice)
}