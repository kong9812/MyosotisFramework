// Copyright (c) 2025 kong9812
#include "subsystem/render/renderSwapchain.h"
#include "vkValidation.h"
#include "vkCreateInfo.h"
#include "appInfo.h"

namespace MyosotisFW::System::Render
{
	RenderSwapchain::RenderSwapchain(RenderDevice_ptr renderDevice, VkSurfaceKHR& vkSurface)
	{
		m_device = renderDevice;

		// surface capabilities
		VkSurfaceCapabilitiesKHR surfCaps{};
		VK_VALIDATION(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->GetPhysicalDevice(), vkSurface, &surfCaps));

		// surfacePresentModes
		uint32_t surfacePresentModesCount = 0;
		VK_VALIDATION(vkGetPhysicalDeviceSurfacePresentModesKHR(m_device->GetPhysicalDevice(), vkSurface, &surfacePresentModesCount, nullptr));
		std::vector<VkPresentModeKHR> surfacePresentModes(surfacePresentModesCount);
		VK_VALIDATION(vkGetPhysicalDeviceSurfacePresentModesKHR(m_device->GetPhysicalDevice(), vkSurface, &surfacePresentModesCount, surfacePresentModes.data()));

		uint32_t surfaceFormatsCount;
		VK_VALIDATION(vkGetPhysicalDeviceSurfaceFormatsKHR(m_device->GetPhysicalDevice(), vkSurface, &surfaceFormatsCount, NULL));
		std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatsCount);
		VK_VALIDATION(vkGetPhysicalDeviceSurfaceFormatsKHR(m_device->GetPhysicalDevice(), vkSurface, &surfaceFormatsCount, surfaceFormats.data()));
		ASSERT(std::find_if(surfaceFormats.begin(), surfaceFormats.end(),
			[](VkSurfaceFormatKHR sf) 
			{
				return ((sf.format == AppInfo::g_surfaceFormat.format) && (sf.colorSpace == AppInfo::g_surfaceFormat.colorSpace)); 
			}) != surfaceFormats.end(), "Could not find a matching surface format!");
		m_width = surfCaps.currentExtent.width;
		m_height = surfCaps.currentExtent.height;

		VkSwapchainCreateInfoKHR swapchainCreateInfo = Utility::Vulkan::CreateInfo::swapchainCreateInfo(
			vkSurface,
			surfCaps,
			AppInfo::g_minImageCount,
			AppInfo::g_surfaceFormat,
			AppInfo::g_presentMode
		);
		VK_VALIDATION(vkCreateSwapchainKHR(*m_device, &swapchainCreateInfo, nullptr, &m_swapchain));

		// swapchain image
		uint32_t imageCount = 0;
		VK_VALIDATION(vkGetSwapchainImagesKHR(*m_device, m_swapchain, &imageCount, NULL));
		std::vector<VkImage> images(imageCount);
		m_swapchainImage.resize(imageCount);
		VK_VALIDATION(vkGetSwapchainImagesKHR(*m_device, m_swapchain, &imageCount, images.data()));
		for (uint32_t i = 0; i < imageCount; i++)
		{
			m_swapchainImage[i].image = images[i];
			VkImageViewCreateInfo imageViewCreateInfoForSwapchain = Utility::Vulkan::CreateInfo::imageViewCreateInfoForSwapchain(m_swapchainImage[i].image, AppInfo::g_surfaceFormat.format);
			VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfoForSwapchain, nullptr, &m_swapchainImage[i].view));
		}
	};

	RenderSwapchain::~RenderSwapchain()
	{
		for (Utility::Vulkan::Struct::Image& image : m_swapchainImage)
		{
			vkDestroyImageView(*m_device, image.view, nullptr);
		}	
		vkDestroySwapchainKHR(*m_device, m_swapchain, nullptr);
	}

	void RenderSwapchain::AcquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t& imageIndex)
	{
		VK_VALIDATION(vkAcquireNextImageKHR(*m_device, m_swapchain, UINT64_MAX, presentCompleteSemaphore, nullptr, &imageIndex));
	}

	void RenderSwapchain::QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore)
	{
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = NULL;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_swapchain;
		presentInfo.pImageIndices = &imageIndex;
		// Check if a wait semaphore has been specified to wait for before presenting the image
		if (waitSemaphore != VK_NULL_HANDLE)
		{
			presentInfo.pWaitSemaphores = &waitSemaphore;
			presentInfo.waitSemaphoreCount = 1;
		}
		VK_VALIDATION(vkQueuePresentKHR(queue, &presentInfo));
	}
}
