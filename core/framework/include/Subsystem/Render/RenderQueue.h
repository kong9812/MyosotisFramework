// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <mutex>
#include "ClassPointer.h"

namespace MyosotisFW::System::Render
{
	class RenderQueue
	{
	public:
		RenderQueue(const VkPhysicalDevice& phyDevice, const VkQueueFlags& queueFlags);
		~RenderQueue() {}

		void Initialize(const VkPhysicalDevice& phyDevice, const VkQueueFlags& queueFlags);
		void Submit(const VkSubmitInfo& submitInfo, const VkFence& fence = VK_NULL_HANDLE);
		void CreateQueueInstance(const VkDevice& device);
		void CreateCommandPool(const VkDevice& device, const VkAllocationCallbacks* pAllocator);
		std::vector<VkCommandBuffer> AllocateCommandBuffers(const VkDevice& device, const uint32_t count = 1);
		void FreeCommandBuffers(const VkDevice& device, std::vector<VkCommandBuffer> commandBuffers);
		void DestroyCommandPool(const VkDevice& device, const VkAllocationCallbacks* pAllocator);
		void FreeSingleUseCommandBuffer(const VkDevice& device, const VkCommandBuffer& commandBuffer);
		VkCommandBuffer AllocateSingleUseCommandBuffer(const VkDevice& device);

		uint32_t GetQueueFamilyIndex() { return m_queueFamilyIndex; }
		VkQueue GetQueue() { return m_queue; }
		void WaitIdle();

	private:
		uint32_t getQueueFamilyIndex(const VkQueueFlags& queueFlags, const std::vector<VkQueueFamilyProperties>& queueFamilyProperties);

		VkQueue m_queue;
		uint32_t m_queueFamilyIndex;
		mutable std::mutex m_mutex;

		VkCommandPool m_commandPool;
	};
	TYPEDEF_SHARED_PTR_ARGS(RenderQueue);
}