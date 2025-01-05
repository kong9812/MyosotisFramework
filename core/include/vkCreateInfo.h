// Copyright (c) 2025 kong9812
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
		VkApplicationInfo ai{};
		ai.sType = VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO;
		ai.pApplicationName = applicationName;
		ai.pEngineName = engineName;
		ai.apiVersion = apiVersion;
		ai.engineVersion = engineVersion;
		return ai;
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
		ci.queueCount = 1;	// とりあえず1
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

	inline VkRenderPassBeginInfo renderPassBeginInfo(VkRenderPass renderPass, uint32_t width, uint32_t height, const std::vector<VkClearValue>& clearValues)
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

	inline VkDescriptorPoolSize descriptorPoolSize(VkDescriptorType descriptorType, uint32_t descriptorCount)
	{
		VkDescriptorPoolSize ps{};
		ps.type = descriptorType;
		ps.descriptorCount = descriptorCount;
		return ps;
	}

	inline VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(const std::vector<VkDescriptorPoolSize>& descriptorPoolSizes, uint32_t maxSets = 1, VkDescriptorPoolCreateFlags flags = 0)
	{
		VkDescriptorPoolCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		ci.flags = flags;
		ci.maxSets = maxSets;
		ci.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
		ci.pPoolSizes = descriptorPoolSizes.data();
		return ci;
	}

	inline VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags shaderStageFlags, uint32_t descriptorCount = 1)
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

	inline VkDescriptorSetAllocateInfo descriptorSetAllocateInfo(VkDescriptorPool descriptorPool, const VkDescriptorSetLayout* descriptorSetLayout, uint32_t descriptorSetCount = 1)
	{
		VkDescriptorSetAllocateInfo ai{};
		ai.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		ai.descriptorPool = descriptorPool;
		ai.descriptorSetCount = descriptorSetCount;
		ai.pSetLayouts = descriptorSetLayout;
		return ai;
	}

	inline VkWriteDescriptorSet writeDescriptorSet(
		VkDescriptorSet descriptorSet,
		uint32_t dstBinding,
		VkDescriptorType descriptorType,
		const VkDescriptorBufferInfo* pBufferInfo,
		uint32_t descriptorCount = 1)
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

	inline VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(const VkDescriptorSetLayout* pSetLayouts, uint32_t setLayoutCount = 1)
	{
		VkPipelineLayoutCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		ci.setLayoutCount = setLayoutCount;
		ci.pSetLayouts = pSetLayouts;
		return ci;
	}

	inline VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(
		VkPrimitiveTopology primitiveTopology,
		VkBool32 primitiveRestartEnable = VK_FALSE)
	{
		VkPipelineInputAssemblyStateCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		ci.topology = primitiveTopology;						// プリミティブトポロジ
		ci.primitiveRestartEnable = primitiveRestartEnable;		// プリミティブリスタートの有効化
		return ci;
	}

	inline VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(
		VkPolygonMode polygonMode, 
		VkCullModeFlags cullMode,
		VkFrontFace frontFace)
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

	inline VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(VkBool32 blendEnable, VkColorComponentFlags colorComponentFlags = VkColorComponentFlagBits::VK_COLOR_COMPONENT_FLAG_BITS_MAX_ENUM)
	{
		VkPipelineColorBlendAttachmentState as{};
		as.blendEnable = blendEnable;										// ブレンディングの有効化
		as.srcColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO;		// ソースカラーのブレンド係数
		as.dstColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO;		// デスティネーションカラーのブレンド係数
		as.colorBlendOp = VkBlendOp::VK_BLEND_OP_ADD;						// カラーブレンドの演算方法
		as.srcAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO;		// ソースアルファのブレンド係数
		as.dstAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO;		// デスティネーションアルファのブレンド係数
		as.alphaBlendOp = VkBlendOp::VK_BLEND_OP_ADD;						// アルファブレンドの演算方法
		as.colorWriteMask = colorComponentFlags;							// 書き込むカラーチャンネル
		return as;
	}

	inline VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(const VkPipelineColorBlendAttachmentState* pAttachments, uint32_t attachmentCount = 1)
	{
		VkPipelineColorBlendStateCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		ci.logicOpEnable = VK_FALSE;					// 論理演算の有効化
		ci.logicOp = VkLogicOp::VK_LOGIC_OP_CLEAR;		// 論理演算の種類
		ci.attachmentCount = attachmentCount;			// アタッチメントの数
		ci.pAttachments = pAttachments;					// 各アタッチメントの設定
		return ci;
	}
}
