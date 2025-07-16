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
	// 前方宣言
	class RenderQueue;
	TYPEDEF_SHARED_PTR_FWD(RenderQueue);

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
		RenderQueue_ptr GetGraphicsQueue() const { return m_graphicsQueue; }
		RenderQueue_ptr GetComputeQueue() const { return m_computeQueue; }
		RenderQueue_ptr GetTransferQueue() const { return m_transferQueue; }
		VkAllocationCallbacks* GetAllocationCallbacks() { return &m_allocationCallbacks; }
		VmaAllocator GetVmaAllocator() const { return m_allocator; }

		void ImageMemoryAllocate(DeviceImage& deviceImage);

	private:
		VkPhysicalDevice m_physicalDevice;
		VkDevice m_device;
		VmaAllocator m_allocator;
		VkAllocationCallbacks m_allocationCallbacks;

		RenderQueue_ptr m_graphicsQueue;
		RenderQueue_ptr m_computeQueue;
		RenderQueue_ptr m_transferQueue;

		VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProperties;

	private:
		uint32_t getMemoryTypeIndex(const uint32_t& typeBits, const VkMemoryPropertyFlags& properties) const;
		void prepareAllocationCallbacks();
		void prepareVMA(const VkInstance& vkInstance);
	};
	TYPEDEF_SHARED_PTR_ARGS(RenderDevice);
}