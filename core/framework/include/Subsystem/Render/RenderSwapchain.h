// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include "ClassPointer.h"
#include "VK_Validation.h"
#include "Image.h"
#include "iglm.h"

namespace MyosotisFW::System::Render
{
	// 前方宣言
	class RenderDevice;
	TYPEDEF_SHARED_PTR_FWD(RenderDevice);

	class RenderSwapchain
	{
	public:
		RenderSwapchain(const RenderDevice_ptr& renderDevice, const VkSurfaceKHR& vkSurface);
		~RenderSwapchain();

		const glm::ivec2 GetScreenSize() const { return m_screenSize; }
		const glm::vec2 GetScreenSizeF() const { return m_screenSizeF; }
		uint32_t GetImageCount() const { return static_cast<uint32_t>(m_swapchainImage.size()); }
		uint32_t GetMinImageCount() const { return m_minImageCount; }
		std::vector<Image>& GetSwapchainImage() { return m_swapchainImage; }

		void AcquireNextImage(VkSemaphore& presentCompleteSemaphore, uint32_t& imageIndex);
		void QueuePresent(const VkQueue& queue, const uint32_t imageIndex, const VkSemaphore& pWaitSemaphores);

		void Resize(const VkSurfaceKHR& vkSurface);

	private:
		void createSwapchain(const VkSurfaceKHR& vkSurface);

		RenderDevice_ptr m_device;
		VkSwapchainKHR m_swapchain;

		glm::ivec2 m_screenSize;
		glm::vec2 m_screenSizeF;

		std::vector<Image> m_swapchainImage;
		uint32_t m_minImageCount;
	};
	TYPEDEF_SHARED_PTR_ARGS(RenderSwapchain);
}