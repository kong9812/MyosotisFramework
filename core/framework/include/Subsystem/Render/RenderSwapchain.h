// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include "ClassPointer.h"
#include "VK_Validation.h"
#include "Structs.h"

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

		uint32_t GetWidth() const { return m_width; }
		uint32_t GetHeight() const { return m_height; }
		float GetWidthF() const { return static_cast<float>(m_width); }
		float GetHeightF() const { return static_cast<float>(m_height); }
		glm::vec2 GetScreenSize() { return glm::vec2(static_cast<float>(m_width), static_cast<float>(m_height)); }
		uint32_t GetImageCount() { return static_cast<uint32_t>(m_swapchainImage.size()); }
		uint32_t GetMinImageCount() { return m_minImageCount; }
		std::vector<Image>& GetSwapchainImage() { return m_swapchainImage; }

		void AcquireNextImage(VkSemaphore& presentCompleteSemaphore, uint32_t& imageIndex);
		void QueuePresent(const VkQueue& queue, const uint32_t& imageIndex, const VkSemaphore& pWaitSemaphores);

		void Resize(const VkSurfaceKHR& vkSurface);

	private:
		void createSwapchain(const VkSurfaceKHR& vkSurface);

		RenderDevice_ptr m_device;
		VkSwapchainKHR m_swapchain;

		uint32_t m_width;
		uint32_t m_height;

		std::vector<Image> m_swapchainImage;
		uint32_t m_minImageCount;
	};
	TYPEDEF_SHARED_PTR_ARGS(RenderSwapchain);
}