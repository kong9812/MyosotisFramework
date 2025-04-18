// Copyright (c) 2025 kong9812
#pragma once
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>
#include "classPointer.h"
#include "Structs.h"
#include "VK_Validation.h"
#include "ivma.h"

namespace MyosotisFW::System::Render
{
	class RenderDevice
	{
	public:
		RenderDevice(const VkInstance& vkInstance);
		~RenderDevice();

		operator VkDevice() const
		{
			return m_device;
		};

		VkPhysicalDevice GetPhysicalDevice() const { return m_physicalDevice; }
		uint32_t GetGraphicsFamilyIndex() const { return m_graphicsFamilyIndex; }
		uint32_t GetComputeFamilyIndex() const { return m_computeFamilyIndex; }
		uint32_t GetTransferFamilyIndex() const { return m_transferFamilyIndex; }
		VkAllocationCallbacks* GetAllocationCallbacks() { return &m_allocationCallbacks; }
		VmaAllocator GetVmaAllocator() const { return m_allocator; }

		void ImageMemoryAllocate(DeviceImage& deviceImage);

	private:
		VkPhysicalDevice m_physicalDevice;
		VkDevice m_device;
		VmaAllocator m_allocator;
		VkAllocationCallbacks m_allocationCallbacks;

		uint32_t m_graphicsFamilyIndex;
		uint32_t m_computeFamilyIndex;
		uint32_t m_transferFamilyIndex;

		std::vector<VkQueueFamilyProperties> m_queueFamilyProperties;
		VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProperties;

	private:
		uint32_t getQueueFamilyIndex(const VkQueueFlags& queueFlags, const std::vector<VkQueueFamilyProperties>& queueFamilyProperties);
		uint32_t getMemoryTypeIndex(const uint32_t& typeBits, const VkMemoryPropertyFlags& properties) const;
		void prepareAllocationCallbacks();
		void prepareVMA(const VkInstance& vkInstance);
	};
	TYPEDEF_SHARED_PTR_ARGS(RenderDevice)
}