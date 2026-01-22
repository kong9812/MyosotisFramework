// Copyright (c) 2025 kong9812
#include "RenderQueue.h"

#include "Logger.h"
#include "VK_CreateInfo.h"
#include "VK_Validation.h"

namespace MyosotisFW::System::Render
{
	RenderQueue::RenderQueue(const VkPhysicalDevice& phyDevice, const VkQueueFlags& queueFlags) :
		m_queue(VK_NULL_HANDLE),
		m_queueFamilyIndex(0),
		m_mutex()
	{
		Initialize(phyDevice, queueFlags);
	}

	void RenderQueue::Initialize(const VkPhysicalDevice& phyDevice, const VkQueueFlags& queueFlags)
	{
		// queue family properties
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(phyDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(phyDevice, &queueFamilyCount, queueFamilyProperties.data());
		m_queueFamilyIndex = getQueueFamilyIndex(queueFlags, queueFamilyProperties);
	}

	void RenderQueue::Submit(const VkSubmitInfo& submitInfo, const VkFence& fence)
	{
		ASSERT(m_queue, "You must call CreateQueueInstance before submit command to queue.");
		std::lock_guard<std::mutex> lock(m_mutex);
		VK_VALIDATION(vkQueueSubmit(m_queue, 1, &submitInfo, fence));
	}

	void RenderQueue::CreateQueueInstance(const VkDevice& device)
	{
		vkGetDeviceQueue(device, m_queueFamilyIndex, 0, &m_queue);
	}

	void RenderQueue::CreateCommandPool(const VkDevice& device, const VkAllocationCallbacks* pAllocator)
	{
		VkCommandPoolCreateInfo cmdPoolInfo = Utility::Vulkan::CreateInfo::commandPoolCreateInfo(m_queueFamilyIndex);
		VK_VALIDATION(vkCreateCommandPool(device, &cmdPoolInfo, pAllocator, &m_commandPool));
	}

	std::vector<VkCommandBuffer> RenderQueue::AllocateCommandBuffers(const VkDevice& device, const uint32_t count) const
	{
		std::vector<VkCommandBuffer> commandBuffers(count);
		VkCommandBufferAllocateInfo cmdBufAllocateInfo = Utility::Vulkan::CreateInfo::commandBufferAllocateInfo(m_commandPool, VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY, count);
		VK_VALIDATION(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, commandBuffers.data()));
		return commandBuffers;
	}

	void RenderQueue::FreeCommandBuffers(const VkDevice& device, std::vector<VkCommandBuffer> commandBuffers) const
	{
		if (commandBuffers.size() > 0)
		{
			vkFreeCommandBuffers(device, m_commandPool, commandBuffers.size(), commandBuffers.data());
		}
	}

	void RenderQueue::DestroyCommandPool(const VkDevice& device, const VkAllocationCallbacks* pAllocator) const
	{
		vkDestroyCommandPool(device, m_commandPool, pAllocator);
	}

	void RenderQueue::FreeSingleUseCommandBuffer(const VkDevice& device, const VkCommandBuffer& commandBuffer) const
	{
		// todo. deviceをメンバー変数として持たせる
		vkFreeCommandBuffers(device, m_commandPool, 1, &commandBuffer);
	}

	VkCommandBuffer RenderQueue::AllocateSingleUseCommandBuffer(const VkDevice& device) const
	{
		VkCommandBuffer commandBuffer;
		VkCommandBufferAllocateInfo cmdBufAllocateInfo = Utility::Vulkan::CreateInfo::commandBufferAllocateInfo(m_commandPool, VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
		VK_VALIDATION(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, &commandBuffer));
		return commandBuffer;
	}

	void RenderQueue::WaitIdle() const
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		VK_VALIDATION(vkQueueWaitIdle(m_queue));
	}

	void RenderQueue::QueuePresent(const VkSwapchainKHR& swapchain, const uint32_t imageIndex, const VkSemaphore& pWaitSemaphores)
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = NULL;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchain;
		presentInfo.pImageIndices = &imageIndex;
		// Check if a wait semaphore has been specified to wait for before presenting the image
		if (pWaitSemaphores != nullptr)
		{
			presentInfo.pWaitSemaphores = &pWaitSemaphores;
			presentInfo.waitSemaphoreCount = 1;
		}
		VkResult result = vkQueuePresentKHR(m_queue, &presentInfo);
		if (result != VkResult::VK_ERROR_OUT_OF_DATE_KHR)	// resize
		{
			VK_VALIDATION(result);
		}
	}

	uint32_t RenderQueue::getQueueFamilyIndex(const VkQueueFlags& queueFlags, const std::vector<VkQueueFamilyProperties>& queueFamilyProperties)
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
}