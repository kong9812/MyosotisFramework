// Copyright (c) 2025 kong9812
#include "BindlessResourcesRenderPipeline.h"
#include "VK_CreateInfo.h"
#include "AppInfo.h"

namespace MyosotisFW::System::Render
{
	BindlessResourcesRenderPipeline::~BindlessResourcesRenderPipeline()
	{
		vkDestroyPipeline(*m_device, m_pipeline, m_device->GetAllocationCallbacks());
		vkDestroyPipelineLayout(*m_device, m_pipelineLayout, m_device->GetAllocationCallbacks());

		vkDestroySampler(*m_device, m_image.sampler, m_device->GetAllocationCallbacks());
	}

	void BindlessResourcesRenderPipeline::Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass)
	{
		prepareRenderPipeline(resources, renderPass);

		m_image = resources->GetImage("NormalMap.png");

		// sampler
		VkSamplerCreateInfo samplerCreateInfo = Utility::Vulkan::CreateInfo::samplerCreateInfo();
		VK_VALIDATION(vkCreateSampler(*m_device, &samplerCreateInfo, m_device->GetAllocationCallbacks(), &m_image.sampler));
	}

	void BindlessResourcesRenderPipeline::BindCommandBuffer(const VkCommandBuffer& commandBuffer)
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_shaderBase.pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_shaderBase.pipelineLayout, 0, 1, &m_shaderBase.descriptorSet, 0, NULL);
		vkCmdPushConstants(commandBuffer, m_shaderBase.pipelineLayout,
			VkShaderStageFlagBits::VK_SHADER_STAGE_ALL,
			0, static_cast<uint32_t>(sizeof(m_pushConstant)), &m_pushConstant);
		vkCmdDraw(commandBuffer, 3, 1, 0, 0);
	}

	void BindlessResourcesRenderPipeline::CreateShaderObject(const glm::vec2& screenSize)
	{
		{// pipeline
			m_shaderBase.pipelineLayout = m_pipelineLayout;
			m_shaderBase.pipeline = m_pipeline;
		}

		m_pushConstant.textureId = 0;
		m_pushConstant.empty = 0;
		m_pushConstant.screenSize = screenSize;

		// layout allocate
		m_shaderBase.descriptorSet = m_descriptors->GetBindlessDescriptorSet();
	}

	void BindlessResourcesRenderPipeline::UpdateDescriptors()
	{
		VkDescriptorImageInfo imageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(m_image.sampler, m_image.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		m_pushConstant.textureId = m_descriptors->AddCombinedImageSamplerInfo(imageInfo);
	}

	void BindlessResourcesRenderPipeline::prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass)
	{
		// push constant
		VkPushConstantRange pushConstantRange = Utility::Vulkan::CreateInfo::pushConstantRange(VkShaderStageFlagBits::VK_SHADER_STAGE_ALL, 0, static_cast<uint32_t>(sizeof(m_pushConstant)));

		// [pipeline]layout
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts = { m_descriptors->GetBindlessDescriptorSetLayout() };
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = Utility::Vulkan::CreateInfo::pipelineLayoutCreateInfo(descriptorSetLayouts);
		pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
		pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
		VK_VALIDATION(vkCreatePipelineLayout(*m_device, &pipelineLayoutCreateInfo, m_device->GetAllocationCallbacks(), &m_pipelineLayout));

		// pipeline
		std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo{
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT, resources->GetShaderModules("BindlessResources.vert.spv")),
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT, resources->GetShaderModules("BindlessResources.frag.spv")),
		};

		// pipelineVertexInputStateCreateInfo
		VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineVertexInputStateCreateInfo();
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineInputAssemblyStateCreateInfo(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		VkPipelineViewportStateCreateInfo viewportStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineViewportStateCreateInfo();
		VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineRasterizationStateCreateInfo(VkPolygonMode::VK_POLYGON_MODE_FILL, VkCullModeFlagBits::VK_CULL_MODE_FRONT_BIT, VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
		VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineMultisampleStateCreateInfo();
		VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VkCompareOp::VK_COMPARE_OP_LESS_OR_EQUAL);
		std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentStates = {
			Utility::Vulkan::CreateInfo::pipelineColorBlendAttachmentState(VK_TRUE),
		};
		VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineColorBlendStateCreateInfo(colorBlendAttachmentStates);
		std::vector<VkDynamicState> dynamicStates = { VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT, VkDynamicState::VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineDynamicStateCreateInfo(dynamicStates);

		VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = Utility::Vulkan::CreateInfo::graphicsPipelineCreateInfo(
			shaderStageCreateInfo,									// シェーダーステージ
			&pipelineVertexInputStateCreateInfo,					// 頂点入力
			&inputAssemblyStateCreateInfo,							// 入力アセンブリ
			&viewportStateCreateInfo,								// ビューポートステート
			&rasterizationStateCreateInfo,							// ラスタライゼーション
			&multisampleStateCreateInfo,							// マルチサンプリング
			&depthStencilStateCreateInfo,							// 深度/ステンシル
			&colorBlendStateCreateInfo,								// カラーブレンディング
			&dynamicStateCreateInfo,								// 動的状態
			m_pipelineLayout,										// パイプラインレイアウト
			renderPass);											// レンダーパス
		graphicsPipelineCreateInfo.subpass = 0;
		VK_VALIDATION(vkCreateGraphicsPipelines(*m_device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, m_device->GetAllocationCallbacks(), &m_pipeline));
	}
}