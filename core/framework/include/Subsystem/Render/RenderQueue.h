// Copyright (c) 2025 kong9812
#pragma once
#include <vector>
#include <vulkan/vulkan.h>
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
		void Submit(const VkSubmitInfo& submitInfo);
		void CreateQueueInstance(const VkDevice& device);

		uint32_t GetQueueFamilyIndex() { return m_queueFamilyIndex; }
		VkQueue GetQueue() { return m_queue; }
		void WaitIdle();

	private:
		uint32_t getQueueFamilyIndex(const VkQueueFlags& queueFlags, const std::vector<VkQueueFamilyProperties>& queueFamilyProperties);

		VkQueue m_queue;
		uint32_t m_queueFamilyIndex;
		mutable std::mutex m_mutex;
	};
	TYPEDEF_SHARED_PTR_ARGS(RenderQueue)
}