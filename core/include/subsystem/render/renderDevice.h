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

		template<typename T>
		void CreateUBOBuffer(Utility::Vulkan::Struct::Buffer& buffer, T ubo)
		{
			// Buffer
			VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::uboBufferCreateInfo(ubo);
			VK_VALIDATION(vkCreateBuffer(m_device, &bufferCreateInfo, nullptr, &buffer.buffer));

			// Memory allocate
			VkMemoryRequirements memReqs{};
			vkGetBufferMemoryRequirements(m_device, buffer.buffer, &memReqs);
			VkMemoryAllocateInfo memoryAllocateInfo = Utility::Vulkan::CreateInfo::memoryAllocateInfo(memReqs.size, getMemoryTypeIndex(memReqs.memoryTypeBits, VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
			VK_VALIDATION(vkAllocateMemory(m_device, &memoryAllocateInfo, nullptr, &buffer.memory));

			// Descriptor buffer info
			buffer.descriptor.buffer = buffer.buffer;   // UBOバッファ
			buffer.descriptor.offset = 0;               // バッファの開始位置（通常は0）
			buffer.descriptor.range = sizeof(ubo);      // UBOデータのサイズ

			// Bind
			VK_VALIDATION(vkBindBufferMemory(m_device, buffer.buffer, buffer.memory, buffer.descriptor.offset));

			// Map
			VK_VALIDATION(vkMapMemory(m_device, buffer.memory, buffer.descriptor.offset, sizeof(ubo), 0, &buffer.mapped));
		}

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