// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include "classPointer.h"
#include "renderDevice.h"
#include "vkStruct.h"

namespace MyosotisFW::System::Render
{
	class RenderSwapchain
	{
	public:
		RenderSwapchain(RenderDevice_ptr renderDevice, VkSurfaceKHR& vkSurface);
		~RenderSwapchain();

		uint32_t GetWidth() { return m_width; }
		uint32_t GetHeight() { return m_height; }
		uint32_t GetImageCount() { return static_cast<uint32_t>(m_swapchainImage.size()); }
		std::vector<Utility::Vulkan::Struct::Image>& GetSwapchainImage() { return m_swapchainImage; }

		void AcquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t& imageIndex);
		void QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore);

	private:
		RenderDevice_ptr m_device;
		VkSwapchainKHR m_swapchain;

		uint32_t m_width;
		uint32_t m_height;

		std::vector<Utility::Vulkan::Struct::Image> m_swapchainImage;
	};
	TYPEDEF_UNIQUE_PTR_ARGS(RenderSwapchain)
}