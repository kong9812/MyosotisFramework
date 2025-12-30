// Copyright (c) 2025 kong9812
#include "RenderSwapchain.h"
#include "RenderDevice.h"
#include "VK_Validation.h"
#include "VK_CreateInfo.h"
#include "AppInfo.h"

namespace MyosotisFW::System::Render
{
	RenderSwapchain::RenderSwapchain(const RenderDevice_ptr& renderDevice, const VkSurfaceKHR& vkSurface) :
		m_device(renderDevice),
		m_swapchain(VK_NULL_HANDLE),
		m_screenSize(glm::ivec2(0)),
		m_screenSizeF(glm::vec2(0.0f)),
		m_swapchainImage(),
		m_minImageCount(0)
	{
		m_device = renderDevice;

		createSwapchain(vkSurface);
	};

	RenderSwapchain::~RenderSwapchain()
	{
		for (Image& image : m_swapchainImage)
		{
			vkDestroyImageView(*m_device, image.view, m_device->GetAllocationCallbacks());
		}
		vkDestroySwapchainKHR(*m_device, m_swapchain, m_device->GetAllocationCallbacks());
	}

	void RenderSwapchain::AcquireNextImage(VkSemaphore& semaphore, uint32_t& imageIndex)
	{
		VK_VALIDATION(vkAcquireNextImageKHR(*m_device, m_swapchain, UINT64_MAX, semaphore, nullptr, &imageIndex));
	}

	void RenderSwapchain::QueuePresent(const VkQueue& queue, const uint32_t imageIndex, const VkSemaphore& pWaitSemaphores)
	{
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = NULL;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_swapchain;
		presentInfo.pImageIndices = &imageIndex;
		// Check if a wait semaphore has been specified to wait for before presenting the image
		if (pWaitSemaphores != nullptr)
		{
			presentInfo.pWaitSemaphores = &pWaitSemaphores;
			presentInfo.waitSemaphoreCount = 1;
		}
		VkResult result = vkQueuePresentKHR(queue, &presentInfo);
		if (result != VkResult::VK_ERROR_OUT_OF_DATE_KHR)	// resize
		{
			VK_VALIDATION(result);
		}
	}

	void RenderSwapchain::Resize(const VkSurfaceKHR& vkSurface)
	{
		for (Image& image : m_swapchainImage)
		{
			vkDestroyImageView(*m_device, image.view, m_device->GetAllocationCallbacks());
		}
		createSwapchain(vkSurface);
	}

	void RenderSwapchain::createSwapchain(const VkSurfaceKHR& vkSurface)
	{
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
		m_screenSize = glm::ivec2(surfCaps.currentExtent.width, surfCaps.currentExtent.height);
		m_screenSizeF = glm::vec2(static_cast<float>(m_screenSize.x), static_cast<float>(m_screenSize.y));

		m_minImageCount = surfCaps.maxImageCount < AppInfo::g_minImageCount ? surfCaps.maxImageCount : AppInfo::g_minImageCount;

		VkSwapchainCreateInfoKHR swapchainCreateInfo = Utility::Vulkan::CreateInfo::swapchainCreateInfo(
			vkSurface,
			surfCaps,
			m_minImageCount,
			AppInfo::g_surfaceFormat,
			AppInfo::g_presentMode
		);
		if (m_swapchain != VK_NULL_HANDLE)
		{
			swapchainCreateInfo.oldSwapchain = m_swapchain;
		}
		VK_VALIDATION(vkCreateSwapchainKHR(*m_device, &swapchainCreateInfo, m_device->GetAllocationCallbacks(), &m_swapchain));

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
			VK_VALIDATION(vkCreateImageView(*m_device, &imageViewCreateInfoForSwapchain, m_device->GetAllocationCallbacks(), &m_swapchainImage[i].view));
		}
	}
}
