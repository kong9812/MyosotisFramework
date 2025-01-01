#pragma once
#include <vector>
#include <vulkan/vulkan.h>

namespace Utility::Vulkan::CreateInfo
{
	inline VkApplicationInfo applicationInfo(
		const char* applicationName,
		const char* engineName,
		const uint32_t apiVersion,
		const uint32_t engineVersion)
	{
		VkApplicationInfo applicationInfo{};
		applicationInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.pApplicationName = applicationName;
		applicationInfo.pEngineName = engineName;
		applicationInfo.apiVersion = apiVersion;
		applicationInfo.engineVersion = engineVersion;
		return applicationInfo;
	}

	inline VkInstanceCreateInfo instanceCreateInfo(VkApplicationInfo applicationInfo, const std::vector<const char*>& enabledExtensionNames, const std::vector<const char*>& enabledLayerNames)
	{
		VkInstanceCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		ci.pApplicationInfo = &applicationInfo;
		ci.enabledLayerCount = static_cast<uint32_t>(enabledLayerNames.size());
		ci.ppEnabledLayerNames = enabledLayerNames.data();
		ci.enabledExtensionCount = static_cast<uint32_t>(enabledExtensionNames.size());
		ci.ppEnabledExtensionNames = enabledExtensionNames.data();
		return ci;
	}

	inline VkDeviceQueueCreateInfo deviceQueueCreateInfo(uint32_t queueFamilyIndex, const float& defaultQueuePriority)
	{
		VkDeviceQueueCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		ci.queueFamilyIndex = queueFamilyIndex;
		ci.queueCount = 1;	// ‚Æ‚è‚ ‚¦‚¸1
		ci.pQueuePriorities = &defaultQueuePriority;
		return ci;
	}

	inline VkDeviceCreateInfo deviceCreateInfo(const std::vector<VkDeviceQueueCreateInfo>& deviceQueueCreateInfos, const std::vector<const char*>& enabledExtensionNames, VkPhysicalDeviceFeatures& physicalDeviceFeatures)
	{
		VkDeviceCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		ci.queueCreateInfoCount = static_cast<uint32_t>(deviceQueueCreateInfos.size());
		ci.pQueueCreateInfos = deviceQueueCreateInfos.data();
		ci.enabledExtensionCount = static_cast<uint32_t>(enabledExtensionNames.size());
		ci.ppEnabledExtensionNames = enabledExtensionNames.data();
		ci.pEnabledFeatures = &physicalDeviceFeatures;
		return ci;
	}

	inline VkSwapchainCreateInfoKHR swapchainCreateInfo(
		VkSurfaceKHR surface,
		VkSurfaceCapabilitiesKHR surfCaps,
		uint32_t minImageCount,
		VkSurfaceFormatKHR surfaceFormat,
		VkPresentModeKHR presentMode)
	{
		VkSwapchainCreateInfoKHR ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		ci.surface = surface;
		ci.minImageCount = (surfCaps.maxImageCount < minImageCount ? surfCaps.maxImageCount : minImageCount);
		ci.imageFormat = surfaceFormat.format;
		ci.imageColorSpace = surfaceFormat.colorSpace;
		ci.imageExtent = surfCaps.currentExtent;
		ci.imageArrayLayers = 1;
		ci.imageUsage = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		ci.imageSharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;
		ci.presentMode = presentMode;
		ci.clipped = VK_TRUE;
		ci.preTransform = surfCaps.currentTransform;
		ci.compositeAlpha = VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		if (surfCaps.supportedUsageFlags & VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
			ci.imageUsage |= VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}
		if (surfCaps.supportedUsageFlags & VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
			ci.imageUsage |= VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}
		return ci;
	}

	inline VkSemaphoreCreateInfo semaphoreCreateInfo()
	{
		VkSemaphoreCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		return ci;
	}

	inline VkSubmitInfo submitInfo(VkPipelineStageFlags& submitPipelineStages, VkSemaphore& presentComplete, VkSemaphore& renderComplete)
	{
		VkSubmitInfo si{};
		si.sType = VkStructureType::VK_STRUCTURE_TYPE_SUBMIT_INFO;
		si.pWaitDstStageMask = &submitPipelineStages;
		si.waitSemaphoreCount = 1;
		si.pWaitSemaphores = &presentComplete;
		si.signalSemaphoreCount = 1;
		si.pSignalSemaphores = &renderComplete;
		return si;
	}

	inline VkImageSubresourceRange defaultImageSubresourceRange(VkImageAspectFlags aspectMask)
	{
		VkImageSubresourceRange sr{};
		sr.aspectMask = aspectMask;
		sr.baseMipLevel = 0;
		sr.levelCount = 1;
		sr.baseArrayLayer = 0;
		sr.layerCount = 1;
		return sr;
	}

	inline VkImageViewCreateInfo imageViewCreateInfoForSwapchain(VkImage image, VkFormat format)
	{
		VkImageViewCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ci.image = image;
		ci.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
		ci.format = format;
		ci.components = {
			VkComponentSwizzle::VK_COMPONENT_SWIZZLE_R,
			VkComponentSwizzle::VK_COMPONENT_SWIZZLE_G,
			VkComponentSwizzle::VK_COMPONENT_SWIZZLE_B,
			VkComponentSwizzle::VK_COMPONENT_SWIZZLE_A
		};
		ci.subresourceRange = defaultImageSubresourceRange(VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT);
		return ci;
	}

	inline VkImageCreateInfo imageCreateInfoForDepthStencil(VkFormat format, uint32_t width, uint32_t height)
	{
		VkImageCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		ci.imageType = VkImageType::VK_IMAGE_TYPE_2D;
		ci.format = format;
		ci.extent.width = width;
		ci.extent.height = height;
		ci.extent.depth = 1;
		ci.mipLevels = 1;
		ci.arrayLayers = 1;
		ci.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
		ci.tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
		ci.usage = VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		return ci;
	}

	inline VkImageViewCreateInfo imageViewCreateInfoForDepthStencil(VkImage image, VkFormat format)
	{
		VkImageViewCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ci.image = image;
		ci.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
		ci.format = format;
		ci.components = {
			VkComponentSwizzle::VK_COMPONENT_SWIZZLE_R,
			VkComponentSwizzle::VK_COMPONENT_SWIZZLE_G,
			VkComponentSwizzle::VK_COMPONENT_SWIZZLE_B,
			VkComponentSwizzle::VK_COMPONENT_SWIZZLE_A
		};
		ci.subresourceRange = defaultImageSubresourceRange(VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT | VkImageAspectFlagBits::VK_IMAGE_ASPECT_STENCIL_BIT);
		return ci;
	}

	inline VkMemoryAllocateInfo memoryAllocateInfo(VkDeviceSize allocationSize, uint32_t memoryTypeIndex)
	{
		VkMemoryAllocateInfo ai{};
		ai.sType = VkStructureType::VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		ai.allocationSize = allocationSize;
		ai.memoryTypeIndex = memoryTypeIndex;
		return ai;
	}

	inline VkAttachmentDescription attachmentDescriptionForColor(VkFormat format)
	{
		VkAttachmentDescription ad{};
		ad.format = format;
		ad.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
		ad.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
		ad.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
		ad.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		ad.stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
		ad.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
		ad.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		return ad;
	}

	inline VkAttachmentDescription attachmentDescriptionForDepthStencil(VkFormat format)
	{
		VkAttachmentDescription ad{};
		ad.format = format;
		ad.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
		ad.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
		ad.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
		ad.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
		ad.stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
		ad.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
		ad.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		return ad;
	}

	inline VkAttachmentReference attachmentReference(uint32_t attachment, VkImageLayout layout)
	{
		VkAttachmentReference ar{};
		ar.attachment = attachment;
		ar.layout = layout;
		return ar;
	}

	inline VkSubpassDescription subpassDescription(VkAttachmentReference& color, VkAttachmentReference& depth)
	{
		VkSubpassDescription sd{};
		sd.pipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
		sd.colorAttachmentCount = 1;
		sd.pColorAttachments = &color;
		sd.pDepthStencilAttachment = &depth;
		sd.inputAttachmentCount = 0;
		sd.preserveAttachmentCount = 0;
		return sd;
	}

	inline VkSubpassDependency subpassDependencyForColor()
	{
		VkSubpassDependency sd{};
		sd.srcSubpass = VK_SUBPASS_EXTERNAL;
		sd.dstSubpass = 0;
		sd.srcStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		sd.dstStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		sd.srcAccessMask = VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		sd.dstAccessMask = VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		sd.dependencyFlags = 0;
		return sd;
	}

	inline VkSubpassDependency subpassDependencyForDepthStencil()
	{
		VkSubpassDependency sd{};
		sd.srcSubpass = VK_SUBPASS_EXTERNAL;
		sd.dstSubpass = 0;
		sd.srcStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		sd.dstStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		sd.srcAccessMask = 0;
		sd.dstAccessMask = VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		sd.dependencyFlags = 0;
		return sd;
	}

	inline VkRenderPassCreateInfo renderPassCreateInfo(
		std::vector<VkAttachmentDescription>& attachments,
		std::vector<VkSubpassDependency>& dependencies,
		std::vector<VkSubpassDescription>& subpasses)
	{
		VkRenderPassCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		ci.attachmentCount = static_cast<uint32_t>(attachments.size());
		ci.pAttachments = attachments.data();
		ci.subpassCount = static_cast<uint32_t>(subpasses.size());
		ci.pSubpasses = subpasses.data();
		ci.dependencyCount = static_cast<uint32_t>(dependencies.size());
		ci.pDependencies = dependencies.data();
		return ci;
	}

	inline VkCommandPoolCreateInfo commandPoolCreateInfo(uint32_t queueFamilyIndex)
	{
		VkCommandPoolCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		ci.flags = VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		ci.queueFamilyIndex = queueFamilyIndex;
		return ci;
	}

	inline VkCommandBufferAllocateInfo commandBufferAllocateInfo(VkCommandPool commandPool, VkCommandBufferLevel level, uint32_t commandBufferCount)
	{
		VkCommandBufferAllocateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		ci.commandPool = commandPool;
		ci.level = level;
		ci.commandBufferCount = commandBufferCount;
		return ci;
	}

	inline VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags)
	{
		VkFenceCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		ci.flags = flags;
		return ci;
	}

	inline VkPipelineCacheCreateInfo pipelineCacheCreateInfo()
	{
		VkPipelineCacheCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		return ci;
	}

	inline VkCommandBufferBeginInfo commandBufferBeginInfo()
	{
		VkCommandBufferBeginInfo bi{};
		bi.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		return bi;
	}

	inline VkRenderPassBeginInfo renderPassBeginInfo(VkRenderPass renderPass, uint32_t width, uint32_t height, std::vector<VkClearValue>& clearValues)
	{
		VkRenderPassBeginInfo bi{};
		bi.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		bi.renderPass = renderPass;
		bi.renderArea.offset.x = 0;
		bi.renderArea.offset.y = 0;
		bi.renderArea.extent.width = width;
		bi.renderArea.extent.height = height;
		bi.clearValueCount = 2;
		bi.pClearValues = clearValues.data();
		return bi;
	}

	inline VkViewport viewport(float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f)
	{
		VkViewport v{};
		v.width = width;
		v.height = height;
		v.minDepth = minDepth;
		v.maxDepth = maxDepth;
		return v;
	}

	inline VkRect2D rect2D(int32_t width, int32_t height, int32_t offsetX = 0, int32_t offsetY = 0)
	{
		VkRect2D r2d{};
		r2d.extent.width = width;
		r2d.extent.height = height;
		r2d.offset.x = offsetX;
		r2d.offset.y = offsetY;
		return r2d;
	}
}
