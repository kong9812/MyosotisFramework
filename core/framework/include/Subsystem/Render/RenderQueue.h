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
		std::vector<VkCommandBuffer> AllocateCommandBuffers(const VkDevice& device, const uint32_t count = 1) const;
		void FreeCommandBuffers(const VkDevice& device, std::vector<VkCommandBuffer> commandBuffers) const;
		void DestroyCommandPool(const VkDevice& device, const VkAllocationCallbacks* pAllocator) const;
		void FreeSingleUseCommandBuffer(const VkDevice& device, const VkCommandBuffer& commandBuffer)  const;
		VkCommandBuffer AllocateSingleUseCommandBuffer(const VkDevice& device) const;

		uint32_t GetQueueFamilyIndex() const { return m_queueFamilyIndex; }
		VkQueue GetQueue() const { return m_queue; }
		void WaitIdle() const;

		std::mutex& GetQueueMutex() const { return m_mutex; }

		void QueuePresent(const VkSwapchainKHR& swapchain, const uint32_t imageIndex, const VkSemaphore& pWaitSemaphores);

	private:
		uint32_t getQueueFamilyIndex(const VkQueueFlags& queueFlags, const std::vector<VkQueueFamilyProperties>& queueFamilyProperties);

		VkQueue m_queue;
		uint32_t m_queueFamilyIndex;
		mutable std::mutex m_mutex;

		VkCommandPool m_commandPool;
	};
	TYPEDEF_SHARED_PTR_ARGS(RenderQueue);
}