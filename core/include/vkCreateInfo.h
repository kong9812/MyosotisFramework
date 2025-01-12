// Copyright (c) 2025 kong9812
#pragma once
#include <vector>
#include <vulkan/vulkan.h>

namespace Utility::Vulkan::CreateInfo
{
	typedef enum VertexAttributeBit {
		POSITION_VEC3	= 0x00000001,
		POSITION_VEC4	= 0x00000002,
		NORMAL			= 0x00000004,
		UV				= 0x00000008,
		TANGENT			= 0x00000010,
		COLOR_VEC3		= 0x00000020,
		COLOR_VEC4		= 0x00000040,
		VERTEX_ATTRIBUTE_BITS_MAX_ENUM = 0x7FFFFFFF
	} VertexAttributeBit;
	typedef uint32_t VertexAttributeBits;

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
		ci.queueCount = 1;	// �Ƃ肠����1
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
		ci.topology = primitiveTopology;						// �v���~�e�B�u�g�|���W
		ci.primitiveRestartEnable = primitiveRestartEnable;		// �v���~�e�B�u���X�^�[�g�̗L����
		return ci;
	}

	inline VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(
		VkPolygonMode polygonMode, 
		VkCullModeFlags cullMode,
		VkFrontFace frontFace)
	{
		VkPipelineRasterizationStateCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		ci.depthClampEnable = VK_FALSE;					// �[�x�N�����v�̗L����
		ci.rasterizerDiscardEnable = VK_FALSE;			// ���X�^���C�[�[�V�����̔j��
		ci.polygonMode = polygonMode;					// �|���S���̓h��Ԃ����[�h
		ci.cullMode = cullMode;							// �J�����O���[�h
		ci.frontFace = frontFace;						// �O�ʂ̔�����@
		ci.depthBiasClamp = VK_FALSE;					// �[�x�o�C�A�X�̗L����
		ci.depthBiasConstantFactor = 0.0f;				// �[�x�o�C�A�X�̒萔
		ci.depthBiasClamp = 0.0f;						// �[�x�o�C�A�X�̍ő�l
		ci.depthBiasSlopeFactor = 0.0f;					// �[�x�o�C�A�X�̃X���[�v�X�P�[��
		ci.lineWidth = 1.0f;							// ���C����
		return ci;
	}

	inline VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(VkBool32 blendEnable, VkColorComponentFlags colorComponentFlags = VkColorComponentFlagBits::VK_COLOR_COMPONENT_FLAG_BITS_MAX_ENUM)
	{
		VkPipelineColorBlendAttachmentState as{};
		as.blendEnable = blendEnable;										// �u�����f�B���O�̗L����
		as.srcColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO;		// �\�[�X�J���[�̃u�����h�W��
		as.dstColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO;		// �f�X�e�B�l�[�V�����J���[�̃u�����h�W��
		as.colorBlendOp = VkBlendOp::VK_BLEND_OP_ADD;						// �J���[�u�����h�̉��Z���@
		as.srcAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO;		// �\�[�X�A���t�@�̃u�����h�W��
		as.dstAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO;		// �f�X�e�B�l�[�V�����A���t�@�̃u�����h�W��
		as.alphaBlendOp = VkBlendOp::VK_BLEND_OP_ADD;						// �A���t�@�u�����h�̉��Z���@
		as.colorWriteMask = colorComponentFlags;							// �������ރJ���[�`�����l��
		return as;
	}

	inline VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(const VkPipelineColorBlendAttachmentState* pAttachments, uint32_t attachmentCount = 1)
	{
		VkPipelineColorBlendStateCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		ci.logicOpEnable = VK_FALSE;					// �_�����Z�̗L����
		ci.logicOp = VkLogicOp::VK_LOGIC_OP_CLEAR;		// �_�����Z�̎��
		ci.attachmentCount = attachmentCount;			// �A�^�b�`�����g�̐�
		ci.pAttachments = pAttachments;					// �e�A�^�b�`�����g�̐ݒ�
		return ci;
	}

	inline VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(uint32_t viewportCount = 1,uint32_t scissorCount =1)
	{
		VkPipelineViewportStateCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		ci.viewportCount = viewportCount;		// �r���[�|�[�g�̐�
		ci.pViewports = nullptr;				// �r���[�|�[�g�z�� (�p�C�v���C���쐬���Ƀr���[�|�[�g�𓮓I�ɐݒ肷��ꍇ�� nullptr)
		ci.scissorCount = scissorCount;			// �V�U�[��`�̐�
		ci.pScissors = nullptr;					// �V�U�[��`�z�� (�p�C�v���C���쐬���ɓ��I�ɐݒ肷��ꍇ�� nullptr)
		return ci;
	}

	inline VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(VkSampleCountFlagBits rasterizationSamples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT)
	{
		VkPipelineMultisampleStateCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		ci.rasterizationSamples = rasterizationSamples;		// �T���v����(VK_SAMPLE_COUNT_1_BIT �}���`�T���v�����O�Ȃ�)
		ci.sampleShadingEnable = VK_FALSE;					// �T���v���V�F�[�f�B���O�̗L����
		ci.minSampleShading = 0.0f;							// �ŏ��T���v���V�F�[�f�B���O��
		ci.pSampleMask = nullptr;							// �T���v���}�X�N
		ci.alphaToCoverageEnable = VK_FALSE;				// �A���t�@�J�o���b�W�̗L����
		ci.alphaToOneEnable = VK_FALSE;						// �A���t�@��1�ɌŒ肷��I�v�V����
		return ci;
	}

	inline VkStencilOpState stencilOpState(
		VkCompareOp compareOp = VkCompareOp::VK_COMPARE_OP_NEVER,
		VkStencilOp failOp = VkStencilOp::VK_STENCIL_OP_KEEP,
		VkStencilOp passOp = VkStencilOp::VK_STENCIL_OP_KEEP,
		VkStencilOp depthFailOp = VkStencilOp::VK_STENCIL_OP_KEEP,
		uint32_t compareMask = 0,
		uint32_t writeMask = 0,
		uint32_t reference = 0)
	{
		VkStencilOpState state{};
		state.failOp = failOp;				// �X�e���V���e�X�g���s���̑���
		state.passOp = passOp;				// �X�e���V���e�X�g�������[�x�e�X�g�������̑���
		state.depthFailOp = depthFailOp;    // �X�e���V���������[�x�e�X�g���s���̑���
		state.compareOp = compareOp;		// �X�e���V����r���Z�q
		state.compareMask = compareMask;    // ��r���̃}�X�N
		state.writeMask = writeMask;		// �������ݎ��̃}�X�N
		state.reference = reference;		// ��r�Ɏg�p����Q�ƒl
		return state;
	}

	inline VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(
		VkBool32 depthTestEnable,
		VkBool32 depthWriteEnable,
		VkCompareOp depthCompareOp,
		VkBool32 depthBoundsTestEnable = VK_FALSE,
		VkBool32 stencilTestEnable = VK_FALSE,
		VkStencilOpState front = stencilOpState(),
		VkStencilOpState back = stencilOpState(VkCompareOp::VK_COMPARE_OP_ALWAYS),
		float minDepthBounds = 0.0f,
		float maxDepthBounds = 0.0f
	)
	{
		VkPipelineDepthStencilStateCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		ci.depthTestEnable = depthTestEnable;				// �[�x�e�X�g�̗L����
		ci.depthWriteEnable = depthWriteEnable;				// �[�x�l�̏������݂̗L���� (�ʏ�:VK_TRUE �����I�u�W�F�N�g:VK_FALSE)
		ci.depthCompareOp = depthCompareOp;					// �[�x�l���r�������
		ci.depthBoundsTestEnable = depthBoundsTestEnable;	// �[�x���E�e�X�g��L���� (minDepthBoundst��maxDepthBounds�̊Ԃɐ[�x�l������ꍇ�̂ݕ`��)
		ci.stencilTestEnable = stencilTestEnable;			// �X�e���V���e�X�g�̗L����
		ci.front = front;									// �X�e���V���e�X�g�̐ݒ� (�\)
		ci.back = back;										// �X�e���V���e�X�g�̐ݒ� (��)
		ci.minDepthBounds = minDepthBounds;					// �[�x���E�e�X�g�̍ŏ��l
		ci.maxDepthBounds = maxDepthBounds;					// �[�x���E�e�X�g�̍ő�l
		return ci;
	}

	inline VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(const std::vector<VkDynamicState>& dynamicStates)
	{
		VkPipelineDynamicStateCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		ci.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());		// ���I��Ԃ̐�
		ci.pDynamicStates = dynamicStates.data();								// ���I��Ԃ̔z��
		return ci;
	}

	inline VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule shaderModule)
	{
		VkPipelineShaderStageCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		ci.stage = stage;
		ci.module = shaderModule;
		ci.pName = "main";
		return ci;
	}

	inline VkVertexInputAttributeDescription vertexInputAttributeDescription(uint32_t binding, uint32_t location, VkFormat format, uint32_t offset)
	{
		VkVertexInputAttributeDescription desc{};
		desc.location = location;
		desc.binding = binding;
		desc.format = format;
		desc.offset = offset;
		return desc;
	}

	inline std::vector<VkVertexInputAttributeDescription>& vertexInputAttributeDescriptiones(uint32_t binding, VertexAttributeBits vertexAttributes)
	{
		std::vector<VkVertexInputAttributeDescription> descs{};
		uint32_t location = 0;
		uint32_t offset = 0;

		// [Vec3]Position
		if ((vertexAttributes & VertexAttributeBit::POSITION_VEC3) == 1)
		{
			descs.push_back(vertexInputAttributeDescription(binding, location, VkFormat::VK_FORMAT_R32G32B32_SFLOAT, offset));
			offset += sizeof(glm::vec3);
			location++;
		}
		// [Vec4]Position
		if ((vertexAttributes & VertexAttributeBit::POSITION_VEC4) == 1)
		{
			descs.push_back(vertexInputAttributeDescription(binding, location, VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT, offset));
			offset += sizeof(glm::vec4);
			location++;
		}
		// [Vec3]Normal
		if ((vertexAttributes & VertexAttributeBit::NORMAL) == 1)
		{
			descs.push_back(vertexInputAttributeDescription(binding, location, VkFormat::VK_FORMAT_R32G32B32_SFLOAT, offset));
			offset += sizeof(glm::vec3);
			location++;
		}
		// [Vec2]UV
		if ((vertexAttributes & VertexAttributeBit::UV) == 1)
		{
			descs.push_back(vertexInputAttributeDescription(binding, location, VkFormat::VK_FORMAT_R32G32_SFLOAT, offset));
			offset += sizeof(glm::vec2);
			location++;
		}
		// [Vec3]Tangent
		if ((vertexAttributes & VertexAttributeBit::TANGENT) == 1)
		{
			descs.push_back(vertexInputAttributeDescription(binding, location, VkFormat::VK_FORMAT_R32G32B32_SFLOAT, offset));
			offset += sizeof(glm::vec3);
			location++;
		}
		// [Vec3]Color
		if ((vertexAttributes & VertexAttributeBit::COLOR_VEC3) == 1)
		{
			descs.push_back(vertexInputAttributeDescription(binding, location, VkFormat::VK_FORMAT_R32G32B32_SFLOAT, offset));
			offset += sizeof(glm::vec3);
			location++;
		}
		// [Vec4]Color
		if ((vertexAttributes & VertexAttributeBit::COLOR_VEC4) == 1)
		{
			descs.push_back(vertexInputAttributeDescription(binding, location, VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT, offset));
			offset += sizeof(glm::vec4);
			location++;
		}
		return descs;
	}

	inline VkVertexInputBindingDescription vertexInputBindingDescription(uint32_t binding, VertexAttributeBits vertexAttributes)
	{
		VkVertexInputBindingDescription desc{};
		desc.binding = binding;
		// [Vec3]Position
		if ((vertexAttributes & VertexAttributeBit::POSITION_VEC3) == 1)
		{
			desc.stride += sizeof(glm::vec3);
		}
		// [Vec4]Position
		if ((vertexAttributes & VertexAttributeBit::POSITION_VEC4) == 1)
		{
			desc.stride += sizeof(glm::vec4);
		}
		// [Vec3]Normal
		if ((vertexAttributes & VertexAttributeBit::NORMAL) == 1)
		{
			desc.stride += sizeof(glm::vec3);
		}
		// [Vec2]UV
		if ((vertexAttributes & VertexAttributeBit::UV) == 1)
		{
			desc.stride += sizeof(glm::vec2);
		}
		// [Vec3]Tangent
		if ((vertexAttributes & VertexAttributeBit::TANGENT) == 1)
		{
			desc.stride += sizeof(glm::vec3);
		}
		// [Vec3]Color
		if ((vertexAttributes & VertexAttributeBit::COLOR_VEC3) == 1)
		{
			desc.stride += sizeof(glm::vec3);
		}
		// [Vec4]Color
		if ((vertexAttributes & VertexAttributeBit::COLOR_VEC4) == 1)
		{
			desc.stride += sizeof(glm::vec4);
		}
		desc.inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;
		return desc;
	}

	inline VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo(
		std::vector<VkVertexInputBindingDescription>& vertexBindingDescriptions,
		std::vector<VkVertexInputAttributeDescription>& vertexInputAttributeDescription)
	{
		VkPipelineVertexInputStateCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		ci.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindingDescriptions.size());
		ci.pVertexBindingDescriptions = vertexBindingDescriptions.data();
		ci.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributeDescription.size());
		ci.pVertexAttributeDescriptions = vertexInputAttributeDescription.data();
		return ci;
	}

	inline VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo(
		std::vector<VkPipelineShaderStageCreateInfo>& shaderStageCreateInfo,
		const VkPipelineVertexInputStateCreateInfo* vertexInputState,
		const VkPipelineInputAssemblyStateCreateInfo* inputAssemblyState,
		const VkPipelineViewportStateCreateInfo* viewportState,
		const VkPipelineRasterizationStateCreateInfo* rasterizationState,
		const VkPipelineMultisampleStateCreateInfo* multisampleState,
		const VkPipelineDepthStencilStateCreateInfo* depthStencilState,
		const VkPipelineColorBlendStateCreateInfo* colorBlendState,
		const VkPipelineDynamicStateCreateInfo* dynamicState,
		VkPipelineLayout pipelineLayout,
		VkRenderPass renderPass,
		const VkPipelineTessellationStateCreateInfo* tessellationState = nullptr)
	{
		VkGraphicsPipelineCreateInfo ci{};
		ci.sType = VkStructureType::VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		ci.stageCount = static_cast<uint32_t>(shaderStageCreateInfo.size());		// �V�F�[�_�[�X�e�[�W��
		ci.pStages = shaderStageCreateInfo.data();									// VkPipelineShaderStageCreateInfo�̔z��
		ci.pVertexInputState = vertexInputState;									// ���_����
		ci.pInputAssemblyState = inputAssemblyState;								// ���̓A�Z���u��
		ci.pTessellationState = tessellationState;									// �e�Z���[�V�������
		ci.pViewportState = viewportState;											// �r���[�|�[�g�X�e�[�g (�r���[�|�[�g&�V�U�[)
		ci.pRasterizationState = rasterizationState;								// ���X�^���C�[�[�V����
		ci.pMultisampleState = multisampleState;									// �}���`�T���v�����O
		ci.pDepthStencilState = depthStencilState;									// �[�x/�X�e���V��
		ci.pColorBlendState = colorBlendState;										// �J���[�u�����f�B���O
		ci.pDynamicState = dynamicState;											// ���I���
		ci.layout = pipelineLayout;													// �p�C�v���C�����C�A�E�g
		ci.renderPass = renderPass;													// �����_�[�p�X
		ci.subpass = 0;																// �T�u�p�X�̃C���f�b�N�X
		ci.basePipelineHandle = VK_NULL_HANDLE;										// �h���p�C�v���C�����g�p���Ȃ�
		ci.basePipelineIndex = -1;
		return ci;
	}

	template<typename T>
	inline VkBufferCreateInfo uboBufferCreateInfo(T data)
	{
		VkBufferCreateInfo ci{};
		ci.sType = VkBufferCreateInfo::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		ci.size = sizeof(data);													// UBO�̃T�C�Y�i�\���̂̃T�C�Y�ɍ��킹��j
		ci.usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;	// UBO�p�̃r�b�g
		ci.sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;				// ���̃L���[�ł̋��L���K�v�Ȃ��ꍇ
		return ci;
	}
}
