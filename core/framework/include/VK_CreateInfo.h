// Copyright (c) 2025 kong9812
#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include "iglm.h"

namespace Utility::Vulkan::CreateInfo
{
	typedef enum VertexAttributeBit {
		POSITION_VEC3 = 0x00000001,
		POSITION_VEC4 = 0x00000002,
		NORMAL = 0x00000004,
		UV = 0x00000008,
		TANGENT = 0x00000010,
		COLOR_VEC3 = 0x00000020,
		COLOR_VEC4 = 0x00000040,
		VERTEX_ATTRIBUTE_BITS_MAX_ENUM = 0x7FFFFFFF
	} VertexAttributeBit;
	typedef uint32_t VertexAttributeBits;

	inline VkApplicationInfo applicationInfo(
		const char* applicationName,
		const char* engineName,
		const uint32_t& apiVersion,
		const uint32_t& engineVersion)
	{
		VkApplicationInfo ai{};
		ai.sType = VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO;
		ai.pApplicationName = applicationName;
		ai.pEngineName = engineName;
		ai.apiVersion = apiVersion;
		ai.engineVersion = engineVersion;
		return ai;
	}

	inline VkInstanceCreateInfo instanceCreateInfo(const VkApplicationInfo& applicationInfo, const std::vector<const char*>& enabledExtensionNames, const std::vector<const char*>& enabledLayerNames)
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

	inline VkDeviceQueueCreateInfo deviceQueueCreateInfo(const uint32_t& queueFamilyIndex, const float& defaultQueuePriority)
	{
		VkDeviceQueueCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		ci.queueFamilyIndex = queueFamilyIndex;
		ci.queueCount = 1;	// とりあえず1
		ci.pQueuePriorities = &defaultQueuePriority;
		return ci;
	}

	inline VkDeviceCreateInfo deviceCreateInfo(const std::vector<VkDeviceQueueCreateInfo>& deviceQueueCreateInfos, const std::vector<const char*>& enabledExtensionNames, const VkPhysicalDeviceFeatures& physicalDeviceFeatures)
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
		const VkSurfaceKHR& surface,
		const VkSurfaceCapabilitiesKHR& surfCaps,
		const uint32_t& minImageCount,
		const VkSurfaceFormatKHR& surfaceFormat,
		const VkPresentModeKHR& presentMode)
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

	inline VkSubmitInfo submitInfo()
	{
		VkSubmitInfo si{};
		si.sType = VkStructureType::VK_STRUCTURE_TYPE_SUBMIT_INFO;
		return si;
	}

	inline VkSubmitInfo submitInfo(const VkPipelineStageFlags& submitPipelineStages, const VkSemaphore& pWaitSemaphores, const VkSemaphore& pSignalSemaphores)
	{
		VkSubmitInfo si{};
		si.sType = VkStructureType::VK_STRUCTURE_TYPE_SUBMIT_INFO;
		si.pWaitDstStageMask = &submitPipelineStages;
		si.waitSemaphoreCount = 1;
		si.pWaitSemaphores = &pWaitSemaphores;
		si.signalSemaphoreCount = 1;
		si.pSignalSemaphores = &pSignalSemaphores;
		return si;
	}

	inline VkImageSubresourceRange defaultImageSubresourceRange(const VkImageAspectFlags& aspectMask)
	{
		VkImageSubresourceRange sr{};
		sr.aspectMask = aspectMask;
		sr.baseMipLevel = 0;
		sr.levelCount = 1;
		sr.baseArrayLayer = 0;
		sr.layerCount = 1;
		return sr;
	}

	inline VkImageCreateInfo imageCreateInfo(const VkFormat& format, const uint32_t& width, const uint32_t& height)
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
		ci.usage = VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		return ci;
	}

	inline VkImageCreateInfo imageCreateInfoForDepthStencil(const VkFormat& format, const uint32_t& width, const uint32_t& height)
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

	inline VkImageCreateInfo imageCreateInfoForAttachment(const VkFormat& format, const uint32_t& width, const uint32_t& height)
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
		ci.usage = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		return ci;
	}

	inline VkImageViewCreateInfo imageViewCreateInfoForSwapchain(const VkImage& image, const VkFormat& format)
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

	inline VkImageViewCreateInfo imageViewCreateInfo(const VkImage& image, const VkFormat& format)
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

	inline VkImageViewCreateInfo imageViewCreateInfoForDepthStencil(const VkImage& image, const VkFormat& format)
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

	inline VkImageViewCreateInfo imageViewCreateInfoForAttachment(const VkImage& image, const VkFormat& format)
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
	inline VkMemoryAllocateInfo memoryAllocateInfo(const VkDeviceSize& allocationSize, const uint32_t& memoryTypeIndex)
	{
		VkMemoryAllocateInfo ai{};
		ai.sType = VkStructureType::VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		ai.allocationSize = allocationSize;
		ai.memoryTypeIndex = memoryTypeIndex;
		return ai;
	}

	inline VkAttachmentDescription attachmentDescriptionForColor(const VkFormat& format)
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

	inline VkAttachmentDescription attachmentDescriptionForAttachment(const VkFormat& format,
		const VkAttachmentLoadOp& loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR,
		const VkAttachmentStoreOp& storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE,
		const VkImageLayout& initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,
		const VkImageLayout& finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		VkAttachmentDescription ad{};
		ad.format = format;
		ad.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
		ad.loadOp = loadOp;
		ad.storeOp = storeOp;
		ad.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		ad.stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
		ad.initialLayout = initialLayout;
		ad.finalLayout = finalLayout;
		return ad;
	}

	inline VkAttachmentDescription attachmentDescriptionForDepthStencil(const VkFormat& format)
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

	inline VkAttachmentDescription attachmentDescriptionForShadowMap(const VkFormat& format)
	{
		VkAttachmentDescription ad{};
		ad.format = format;
		ad.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
		ad.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
		ad.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
		ad.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
		ad.stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
		ad.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
		ad.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		return ad;
	}

	inline VkAttachmentReference attachmentReference(const uint32_t& attachment, const VkImageLayout& layout)
	{
		VkAttachmentReference ar{};
		ar.attachment = attachment;
		ar.layout = layout;
		return ar;
	}

	inline VkSubpassDescription subpassDescription_colors_inputs_depth(
		const std::vector<VkAttachmentReference>& colorAttachments,
		const VkAttachmentReference& depthStencilAttachment,
		const std::vector<VkAttachmentReference>& inputAttachments)
	{
		VkSubpassDescription sd{};
		sd.pipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
		sd.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size());
		sd.pColorAttachments = colorAttachments.data();
		sd.pDepthStencilAttachment = &depthStencilAttachment;
		sd.inputAttachmentCount = static_cast<uint32_t>(inputAttachments.size());
		sd.pInputAttachments = inputAttachments.data();
		return sd;
	}

	inline VkSubpassDescription subpassDescription_color_input(
		const VkAttachmentReference& colorAttachment,
		const VkAttachmentReference& inputAttachment)
	{
		VkSubpassDescription sd{};
		sd.pipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
		sd.colorAttachmentCount = 1;
		sd.pColorAttachments = &colorAttachment;
		sd.inputAttachmentCount = 1;
		sd.pInputAttachments = &inputAttachment;
		return sd;
	}

	inline VkSubpassDescription subpassDescription_colors_depth(
		const std::vector<VkAttachmentReference>& colorAttachments,
		const VkAttachmentReference& depthStencilAttachment)
	{
		VkSubpassDescription sd{};
		sd.pipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
		sd.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size());
		sd.pColorAttachments = colorAttachments.data();
		sd.pDepthStencilAttachment = &depthStencilAttachment;
		return sd;
	}

	inline VkSubpassDescription subpassDescription_color_inputs(
		const VkAttachmentReference& colorAttachment,
		const std::vector<VkAttachmentReference>& inputAttachments)
	{
		VkSubpassDescription sd{};
		sd.pipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
		sd.colorAttachmentCount = 1;
		sd.pColorAttachments = &colorAttachment;
		sd.inputAttachmentCount = static_cast<uint32_t>(inputAttachments.size());
		sd.pInputAttachments = inputAttachments.data();
		return sd;
	}

	inline VkSubpassDescription subpassDescription_depth(
		const VkAttachmentReference& depthStencilAttachment)
	{
		VkSubpassDescription sd{};
		sd.pipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
		sd.pDepthStencilAttachment = &depthStencilAttachment;
		return sd;
	}

	inline VkSubpassDescription subpassDescription_color(
		const VkAttachmentReference& colorAttachment)
	{
		VkSubpassDescription sd{};
		sd.pipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
		sd.colorAttachmentCount = 1;
		sd.pColorAttachments = &colorAttachment;
		return sd;
	}

	inline VkSubpassDependency subpassDependency(
		const uint32_t& srcSubpass,
		const uint32_t& dstSubpass,
		const VkPipelineStageFlags& srcStageMask,
		const VkPipelineStageFlags& dstStageMask,
		const VkAccessFlags& srcAccessMask,
		const VkAccessFlags& dstAccessMask,
		const VkDependencyFlags& dependencyFlags
	)
	{
		VkSubpassDependency sd{};
		sd.srcSubpass = srcSubpass;				// ソースサブパスインデックス
		sd.dstSubpass = dstSubpass;				// 宛先サブパスインデックス
		sd.srcStageMask = srcStageMask;			// ソースサブパスの開始条件
		sd.dstStageMask = dstStageMask;			// 宛先サブパスの開始条件
		sd.srcAccessMask = srcAccessMask;		// ソースサブパスが持っているアクセスタイプ
		sd.dstAccessMask = dstAccessMask;		// 宛先サブパスが持っているアクセスタイプ
		sd.dependencyFlags = dependencyFlags;	// 依存関係の挙動を制御するフラグ
		return sd;
	}

	inline VkRenderPassCreateInfo renderPassCreateInfo(
		const std::vector<VkAttachmentDescription>& attachments,
		const std::vector<VkSubpassDependency>& dependencies,
		const std::vector<VkSubpassDescription>& subpasses)
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

	inline VkRenderPassCreateInfo renderPassCreateInfo(
		const std::vector<VkAttachmentDescription>& attachments,
		const std::vector<VkSubpassDependency>& dependencies,
		const VkSubpassDescription& subpasses)
	{
		VkRenderPassCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		ci.attachmentCount = static_cast<uint32_t>(attachments.size());
		ci.pAttachments = attachments.data();
		ci.subpassCount = 1;
		ci.pSubpasses = &subpasses;
		ci.dependencyCount = static_cast<uint32_t>(dependencies.size());
		ci.pDependencies = dependencies.data();
		return ci;
	}

	inline VkCommandPoolCreateInfo commandPoolCreateInfo(const uint32_t& queueFamilyIndex)
	{
		VkCommandPoolCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		ci.flags = VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		ci.queueFamilyIndex = queueFamilyIndex;
		return ci;
	}

	inline VkCommandBufferAllocateInfo commandBufferAllocateInfo(const VkCommandPool& commandPool, const VkCommandBufferLevel& level, const uint32_t& commandBufferCount)
	{
		VkCommandBufferAllocateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		ci.commandPool = commandPool;
		ci.level = level;
		ci.commandBufferCount = commandBufferCount;
		return ci;
	}

	inline VkFenceCreateInfo fenceCreateInfo()
	{
		VkFenceCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		return ci;
	}

	inline VkFenceCreateInfo fenceCreateInfo(const VkFenceCreateFlags& flags)
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

	inline VkRenderPassBeginInfo renderPassBeginInfo(const VkRenderPass& renderPass, const uint32_t& width, const uint32_t& height, const std::vector<VkClearValue>& clearValues)
	{
		VkRenderPassBeginInfo bi{};
		bi.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		bi.renderPass = renderPass;
		bi.renderArea.offset.x = 0;
		bi.renderArea.offset.y = 0;
		bi.renderArea.extent.width = width;
		bi.renderArea.extent.height = height;
		bi.clearValueCount = static_cast<uint32_t>(clearValues.size());
		bi.pClearValues = clearValues.data();
		return bi;
	}

	inline VkViewport viewport(const float& width, const float& height, const float& minDepth = 0.0f, const float& maxDepth = 1.0f)
	{
		VkViewport v{};
		v.width = width;
		v.height = height;
		v.minDepth = minDepth;
		v.maxDepth = maxDepth;
		return v;
	}

	inline VkRect2D rect2D(const int32_t& width, const int32_t& height, const int32_t& offsetX = 0, const int32_t& offsetY = 0)
	{
		VkRect2D r2d{};
		r2d.extent.width = width;
		r2d.extent.height = height;
		r2d.offset.x = offsetX;
		r2d.offset.y = offsetY;
		return r2d;
	}

	inline VkDescriptorPoolSize descriptorPoolSize(const VkDescriptorType& descriptorType, const uint32_t& descriptorCount)
	{
		VkDescriptorPoolSize ps{};
		ps.type = descriptorType;
		ps.descriptorCount = descriptorCount;
		return ps;
	}

	inline VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(const std::vector<VkDescriptorPoolSize>& descriptorPoolSizes, const uint32_t& maxSets = 1, const VkDescriptorPoolCreateFlags& flags = 0)
	{
		VkDescriptorPoolCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		ci.flags = flags;
		ci.maxSets = maxSets;
		ci.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
		ci.pPoolSizes = descriptorPoolSizes.data();
		return ci;
	}

	inline VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(const uint32_t& binding, const VkDescriptorType& descriptorType, const VkShaderStageFlags& shaderStageFlags, const uint32_t& descriptorCount = 1)
	{
		VkDescriptorSetLayoutBinding lb{};
		lb.binding = binding;
		lb.descriptorType = descriptorType;
		lb.descriptorCount = descriptorCount;
		lb.stageFlags = shaderStageFlags;
		return lb;
	}

	inline VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(const std::vector<VkDescriptorSetLayoutBinding>& descriptorSetLayoutBinding)
	{
		VkDescriptorSetLayoutCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		ci.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBinding.size());
		ci.pBindings = descriptorSetLayoutBinding.data();
		return ci;
	}

	inline VkDescriptorSetAllocateInfo descriptorSetAllocateInfo(const VkDescriptorPool& descriptorPool, const VkDescriptorSetLayout* descriptorSetLayout, const uint32_t& descriptorSetCount = 1)
	{
		VkDescriptorSetAllocateInfo ai{};
		ai.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		ai.descriptorPool = descriptorPool;
		ai.descriptorSetCount = descriptorSetCount;
		ai.pSetLayouts = descriptorSetLayout;
		return ai;
	}

	inline VkWriteDescriptorSet writeDescriptorSet(
		const VkDescriptorSet& descriptorSet,
		const uint32_t& dstBinding,
		const VkDescriptorType& descriptorType,
		const VkDescriptorBufferInfo* pBufferInfo,
		const uint32_t& descriptorCount = 1)
	{
		VkWriteDescriptorSet wds{};
		wds.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		wds.dstSet = descriptorSet;
		wds.dstBinding = dstBinding;
		wds.descriptorCount = descriptorCount;
		wds.descriptorType = descriptorType;
		wds.pBufferInfo = pBufferInfo;
		return wds;
	}

	inline VkWriteDescriptorSet writeDescriptorSet(
		const VkDescriptorSet& descriptorSet,
		const uint32_t& dstBinding,
		const VkDescriptorType& descriptorType,
		const VkDescriptorImageInfo* pImageInfo,
		const uint32_t& descriptorCount = 1,
		const uint32_t& resourceID = 0)
	{
		VkWriteDescriptorSet wds{};
		wds.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		wds.dstSet = descriptorSet;
		wds.dstBinding = dstBinding;
		wds.descriptorCount = descriptorCount;
		wds.descriptorType = descriptorType;
		wds.pImageInfo = pImageInfo;
		wds.dstArrayElement = resourceID;
		return wds;
	}

	inline VkDescriptorImageInfo descriptorImageInfo(const VkSampler& sampler, const VkImageView& imageView, const VkImageLayout& imageLayout)
	{
		VkDescriptorImageInfo imageInfo{};
		imageInfo.sampler = sampler;
		imageInfo.imageView = imageView;
		imageInfo.imageLayout = imageLayout;
		return imageInfo;
	}

	inline VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(const std::vector<VkDescriptorSetLayout>& setLayouts)
	{
		VkPipelineLayoutCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		ci.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
		ci.pSetLayouts = setLayouts.data();
		return ci;
	}

	inline VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(
		const VkPrimitiveTopology& primitiveTopology,
		const VkBool32& primitiveRestartEnable = VK_FALSE)
	{
		VkPipelineInputAssemblyStateCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		ci.topology = primitiveTopology;						// プリミティブトポロジ
		ci.primitiveRestartEnable = primitiveRestartEnable;		// プリミティブリスタートの有効化
		return ci;
	}

	inline VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(
		const VkPolygonMode& polygonMode,
		const VkCullModeFlags& cullMode,
		const VkFrontFace& frontFace)
	{
		VkPipelineRasterizationStateCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		ci.depthClampEnable = VK_FALSE;					// 深度クランプの有効化
		ci.rasterizerDiscardEnable = VK_FALSE;			// ラスタライゼーションの破棄
		ci.polygonMode = polygonMode;					// ポリゴンの塗りつぶしモード
		ci.cullMode = cullMode;							// カリングモード
		ci.frontFace = frontFace;						// 前面の判定方法
		ci.depthBiasClamp = VK_FALSE;					// 深度バイアスの有効化
		ci.depthBiasConstantFactor = 0.0f;				// 深度バイアスの定数
		ci.depthBiasClamp = 0.0f;						// 深度バイアスの最大値
		ci.depthBiasSlopeFactor = 0.0f;					// 深度バイアスのスロープスケール
		ci.lineWidth = 1.0f;							// ライン幅
		return ci;
	}

	inline VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(
		const VkBool32& blendEnable,
		const VkColorComponentFlags& colorComponentFlags =
		VkColorComponentFlagBits::VK_COLOR_COMPONENT_R_BIT |
		VkColorComponentFlagBits::VK_COLOR_COMPONENT_G_BIT |
		VkColorComponentFlagBits::VK_COLOR_COMPONENT_B_BIT |
		VkColorComponentFlagBits::VK_COLOR_COMPONENT_A_BIT)
	{
		VkPipelineColorBlendAttachmentState as{};
		as.blendEnable = blendEnable;										// ブレンディングの有効化
		as.srcColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_SRC_ALPHA;		// ソースカラーのブレンド係数
		as.dstColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;		// デスティネーションカラーのブレンド係数
		as.colorBlendOp = VkBlendOp::VK_BLEND_OP_ADD;						// カラーブレンドの演算方法
		as.srcAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_SRC_ALPHA;		// ソースアルファのブレンド係数
		as.dstAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;		// デスティネーションアルファのブレンド係数
		as.alphaBlendOp = VkBlendOp::VK_BLEND_OP_ADD;						// アルファブレンドの演算方法
		as.colorWriteMask = colorComponentFlags;							// 書き込むカラーチャンネル
		return as;
	}

	inline VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo()
	{
		VkPipelineColorBlendStateCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		ci.logicOpEnable = VK_FALSE;										// 論理演算の有効化
		ci.logicOp = VkLogicOp::VK_LOGIC_OP_CLEAR;							// 論理演算の種類
		return ci;
	}

	inline VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(const std::vector<VkPipelineColorBlendAttachmentState>& attachments)
	{
		VkPipelineColorBlendStateCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		ci.logicOpEnable = VK_FALSE;										// 論理演算の有効化
		ci.logicOp = VkLogicOp::VK_LOGIC_OP_CLEAR;							// 論理演算の種類
		ci.attachmentCount = static_cast<uint32_t>(attachments.size());		// アタッチメントの数
		ci.pAttachments = attachments.data();								// 各アタッチメントの設定
		return ci;
	}

	inline VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(const uint32_t& viewportCount = 1, const uint32_t& scissorCount = 1)
	{
		VkPipelineViewportStateCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		ci.viewportCount = viewportCount;		// ビューポートの数
		ci.pViewports = nullptr;				// ビューポート配列 (パイプライン作成時にビューポートを動的に設定する場合は nullptr)
		ci.scissorCount = scissorCount;			// シザー矩形の数
		ci.pScissors = nullptr;					// シザー矩形配列 (パイプライン作成時に動的に設定する場合は nullptr)
		return ci;
	}

	inline VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(const VkSampleCountFlagBits& rasterizationSamples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT)
	{
		VkPipelineMultisampleStateCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		ci.rasterizationSamples = rasterizationSamples;		// サンプル数(VK_SAMPLE_COUNT_1_BIT マルチサンプリングなし)
		ci.sampleShadingEnable = VK_FALSE;					// サンプルシェーディングの有効化
		ci.minSampleShading = 0.0f;							// 最小サンプルシェーディング率
		ci.pSampleMask = nullptr;							// サンプルマスク
		ci.alphaToCoverageEnable = VK_FALSE;				// アルファカバレッジの有効化
		ci.alphaToOneEnable = VK_FALSE;						// アルファを1に固定するオプション
		return ci;
	}

	inline VkStencilOpState stencilOpState(
		const VkCompareOp& compareOp = VkCompareOp::VK_COMPARE_OP_NEVER,
		const VkStencilOp& failOp = VkStencilOp::VK_STENCIL_OP_KEEP,
		const VkStencilOp& passOp = VkStencilOp::VK_STENCIL_OP_KEEP,
		const VkStencilOp& depthFailOp = VkStencilOp::VK_STENCIL_OP_KEEP,
		const uint32_t& compareMask = 0,
		const uint32_t& writeMask = 0,
		const uint32_t& reference = 0)
	{
		VkStencilOpState state{};
		state.failOp = failOp;				// ステンシルテスト失敗時の操作
		state.passOp = passOp;				// ステンシルテスト成功＆深度テスト成功時の操作
		state.depthFailOp = depthFailOp;    // ステンシル成功＆深度テスト失敗時の操作
		state.compareOp = compareOp;		// ステンシル比較演算子
		state.compareMask = compareMask;    // 比較時のマスク
		state.writeMask = writeMask;		// 書き込み時のマスク
		state.reference = reference;		// 比較に使用する参照値
		return state;
	}

	inline VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(
		const VkBool32& depthTestEnable,
		const VkBool32& depthWriteEnable,
		const VkCompareOp& depthCompareOp,
		const VkBool32& depthBoundsTestEnable = VK_FALSE,
		const VkBool32& stencilTestEnable = VK_FALSE,
		const VkStencilOpState& front = stencilOpState(),
		const VkStencilOpState& back = stencilOpState(VkCompareOp::VK_COMPARE_OP_ALWAYS),
		const float& minDepthBounds = 0.0f,
		const float& maxDepthBounds = 0.0f
	)
	{
		VkPipelineDepthStencilStateCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		ci.depthTestEnable = depthTestEnable;				// 深度テストの有効化
		ci.depthWriteEnable = depthWriteEnable;				// 深度値の書き込みの有効化 (通常:VK_TRUE 透明オブジェクト:VK_FALSE)
		ci.depthCompareOp = depthCompareOp;					// 深度値を比較する条件
		ci.depthBoundsTestEnable = depthBoundsTestEnable;	// 深度境界テストを有効化 (minDepthBoundstとmaxDepthBoundsの間に深度値がある場合のみ描画)
		ci.stencilTestEnable = stencilTestEnable;			// ステンシルテストの有効化
		ci.front = front;									// ステンシルテストの設定 (表)
		ci.back = back;										// ステンシルテストの設定 (裏)
		ci.minDepthBounds = minDepthBounds;					// 深度境界テストの最小値
		ci.maxDepthBounds = maxDepthBounds;					// 深度境界テストの最大値
		return ci;
	}

	inline VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(const std::vector<VkDynamicState>& dynamicStates)
	{
		VkPipelineDynamicStateCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		ci.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());		// 動的状態の数
		ci.pDynamicStates = dynamicStates.data();								// 動的状態の配列
		return ci;
	}

	inline VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo(const VkShaderStageFlagBits& stage, const VkShaderModule& shaderModule)
	{
		VkPipelineShaderStageCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		ci.stage = stage;
		ci.module = shaderModule;
		ci.pName = "main";
		return ci;
	}

	inline VkVertexInputAttributeDescription vertexInputAttributeDescription(const uint32_t& binding, const uint32_t& location, const VkFormat& format, const uint32_t& offset)
	{
		VkVertexInputAttributeDescription desc{};
		desc.location = location;
		desc.binding = binding;
		desc.format = format;
		desc.offset = offset;
		return desc;
	}

	inline std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptiones(const uint32_t& binding, const VertexAttributeBits& vertexAttributes)
	{
		std::vector<VkVertexInputAttributeDescription> descs{};
		uint32_t location = 0;
		uint32_t offset = 0;

		// [Vec3]Position
		if (vertexAttributes & VertexAttributeBit::POSITION_VEC3)
		{
			descs.push_back(vertexInputAttributeDescription(binding, location, VkFormat::VK_FORMAT_R32G32B32_SFLOAT, offset));
			offset += sizeof(glm::vec3);
			location++;
		}
		// [Vec4]Position
		if (vertexAttributes & VertexAttributeBit::POSITION_VEC4)
		{
			descs.push_back(vertexInputAttributeDescription(binding, location, VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT, offset));
			offset += sizeof(glm::vec4);
			location++;
		}
		// [Vec3]Normal
		if (vertexAttributes & VertexAttributeBit::NORMAL)
		{
			descs.push_back(vertexInputAttributeDescription(binding, location, VkFormat::VK_FORMAT_R32G32B32_SFLOAT, offset));
			offset += sizeof(glm::vec3);
			location++;
		}
		// [Vec2]UV
		if (vertexAttributes & VertexAttributeBit::UV)
		{
			descs.push_back(vertexInputAttributeDescription(binding, location, VkFormat::VK_FORMAT_R32G32_SFLOAT, offset));
			offset += sizeof(glm::vec2);
			location++;
		}
		// [Vec3]Tangent
		if (vertexAttributes & VertexAttributeBit::TANGENT)
		{
			descs.push_back(vertexInputAttributeDescription(binding, location, VkFormat::VK_FORMAT_R32G32B32_SFLOAT, offset));
			offset += sizeof(glm::vec3);
			location++;
		}
		// [Vec3]Color
		if (vertexAttributes & VertexAttributeBit::COLOR_VEC3)
		{
			descs.push_back(vertexInputAttributeDescription(binding, location, VkFormat::VK_FORMAT_R32G32B32_SFLOAT, offset));
			offset += sizeof(glm::vec3);
			location++;
		}
		// [Vec4]Color
		if (vertexAttributes & VertexAttributeBit::COLOR_VEC4)
		{
			descs.push_back(vertexInputAttributeDescription(binding, location, VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT, offset));
			offset += sizeof(glm::vec4);
			location++;
		}
		return descs;
	}

	inline VkVertexInputBindingDescription vertexInputBindingDescription(const uint32_t& binding, const VertexAttributeBits& vertexAttributes)
	{
		VkVertexInputBindingDescription desc{};
		desc.binding = binding;
		// [Vec3]Position
		if (vertexAttributes & VertexAttributeBit::POSITION_VEC3)
		{
			desc.stride += sizeof(glm::vec3);
		}
		// [Vec4]Position
		if (vertexAttributes & VertexAttributeBit::POSITION_VEC4)
		{
			desc.stride += sizeof(glm::vec4);
		}
		// [Vec3]Normal
		if (vertexAttributes & VertexAttributeBit::NORMAL)
		{
			desc.stride += sizeof(glm::vec3);
		}
		// [Vec2]UV
		if (vertexAttributes & VertexAttributeBit::UV)
		{
			desc.stride += sizeof(glm::vec2);
		}
		// [Vec3]Tangent
		if (vertexAttributes & VertexAttributeBit::TANGENT)
		{
			desc.stride += sizeof(glm::vec3);
		}
		// [Vec3]Color
		if (vertexAttributes & VertexAttributeBit::COLOR_VEC3)
		{
			desc.stride += sizeof(glm::vec3);
		}
		// [Vec4]Color
		if (vertexAttributes & VertexAttributeBit::COLOR_VEC4)
		{
			desc.stride += sizeof(glm::vec4);
		}
		desc.inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;
		return desc;
	}

	inline VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo(
		const std::vector<VkVertexInputBindingDescription>& vertexBindingDescriptions,
		const std::vector<VkVertexInputAttributeDescription>& vertexInputAttributeDescription)
	{
		VkPipelineVertexInputStateCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		ci.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindingDescriptions.size());
		ci.pVertexBindingDescriptions = vertexBindingDescriptions.data();
		ci.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributeDescription.size());
		ci.pVertexAttributeDescriptions = vertexInputAttributeDescription.data();
		return ci;
	}

	inline VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo()
	{
		VkPipelineVertexInputStateCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		return ci;
	}

	inline VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo(
		const std::vector<VkPipelineShaderStageCreateInfo>& shaderStageCreateInfo,
		const VkPipelineVertexInputStateCreateInfo* vertexInputState,
		const VkPipelineInputAssemblyStateCreateInfo* inputAssemblyState,
		const VkPipelineViewportStateCreateInfo* viewportState,
		const VkPipelineRasterizationStateCreateInfo* rasterizationState,
		const VkPipelineMultisampleStateCreateInfo* multisampleState,
		const VkPipelineDepthStencilStateCreateInfo* depthStencilState,
		const VkPipelineColorBlendStateCreateInfo* colorBlendState,
		const VkPipelineDynamicStateCreateInfo* dynamicState,
		const VkPipelineLayout pipelineLayout,
		const VkRenderPass renderPass,
		const VkPipelineTessellationStateCreateInfo* tessellationState = nullptr)
	{
		VkGraphicsPipelineCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		ci.stageCount = static_cast<uint32_t>(shaderStageCreateInfo.size());		// シェーダーステージ数
		ci.pStages = shaderStageCreateInfo.data();									// VkPipelineShaderStageCreateInfoの配列
		ci.pVertexInputState = vertexInputState;									// 頂点入力
		ci.pInputAssemblyState = inputAssemblyState;								// 入力アセンブリ
		ci.pTessellationState = tessellationState;									// テセレーション状態
		ci.pViewportState = viewportState;											// ビューポートステート (ビューポート&シザー)
		ci.pRasterizationState = rasterizationState;								// ラスタライゼーション
		ci.pMultisampleState = multisampleState;									// マルチサンプリング
		ci.pDepthStencilState = depthStencilState;									// 深度/ステンシル
		ci.pColorBlendState = colorBlendState;										// カラーブレンディング
		ci.pDynamicState = dynamicState;											// 動的状態
		ci.layout = pipelineLayout;													// パイプラインレイアウト
		ci.renderPass = renderPass;													// レンダーパス
		ci.subpass = 0;																// サブパスのインデックス
		ci.basePipelineHandle = VK_NULL_HANDLE;										// 派生パイプラインを使用しない
		ci.basePipelineIndex = -1;
		return ci;
	}

	inline VkComputePipelineCreateInfo computePipelineCreateInfo(const VkPipelineLayout& layout, const VkPipelineShaderStageCreateInfo& pStage)
	{
		VkComputePipelineCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		ci.layout = layout;
		ci.stage = pStage;
		return ci;
	}

	inline VkBufferCreateInfo bufferCreateInfo(const uint32_t& size, const VkBufferUsageFlagBits& usage, const VkSharingMode& sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE)
	{
		VkBufferCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		ci.size = size;													// UBOのサイズ（構造体のサイズに合わせる）
		ci.usage = usage;														// UBO用のビット
		ci.sharingMode = sharingMode;											// 他のキューでの共有が必要ない場合
		return ci;
	}

	inline VkDescriptorBufferInfo descriptorBufferInfo(const VkBuffer& buffer, const VkDeviceSize& offset = 0, const VkDeviceSize& range = VK_WHOLE_SIZE)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = buffer;
		bufferInfo.offset = offset;
		bufferInfo.range = range;
		return bufferInfo;
	}

	inline VkDebugUtilsLabelEXT debugUtilsLabelEXT(const glm::vec3& color, const char* labelName)
	{
		VkDebugUtilsLabelEXT label{};
		label.sType = VkStructureType::VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
		label.color[0] = color.r;
		label.color[1] = color.g;
		label.color[2] = color.b;
		label.color[3] = 1.0f;
		label.pLabelName = labelName;
		return label;
	}

	inline VkBufferImageCopy bufferImageCopy(const uint32_t&
		width, const uint32_t& height)
	{
		VkBufferImageCopy imageCopy{};
		imageCopy.bufferOffset = 0;
		imageCopy.bufferRowLength = 0;
		imageCopy.bufferImageHeight = 0;

		imageCopy.imageSubresource.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
		imageCopy.imageSubresource.mipLevel = 0;
		imageCopy.imageSubresource.baseArrayLayer = 0;
		imageCopy.imageSubresource.layerCount = 1;
		imageCopy.imageExtent.depth = 1;
		imageCopy.imageExtent.width = width;
		imageCopy.imageExtent.height = height;
		return imageCopy;
	}

	inline VkSamplerCreateInfo samplerCreateInfo()
	{
		VkSamplerCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		ci.maxAnisotropy = 1.0f;
		ci.magFilter = VK_FILTER_LINEAR;
		ci.minFilter = VK_FILTER_LINEAR;
		ci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		ci.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		ci.addressModeV = ci.addressModeU;
		ci.addressModeW = ci.addressModeU;
		ci.compareEnable = VK_TRUE;
		ci.mipLodBias = 0.0f;
		ci.maxAnisotropy = 1.0f;
		ci.minLod = 0.0f;
		ci.maxLod = 1.0f;
		ci.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		return ci;
	}

	inline VkPushConstantRange pushConstantRange(const VkShaderStageFlags& shaderStageFlags, const uint32_t& offset, const uint32_t& size)
	{
		VkPushConstantRange pc{};
		pc.stageFlags = shaderStageFlags;
		pc.offset = offset;
		pc.size = size;
		return pc;
	}
}
