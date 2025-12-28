// Copyright (c) 2025 kong9812
#include "LightingPipeline.h"
#include "VK_CreateInfo.h"
#include "AppInfo.h"

#include "PrimitiveGeometryShape.h"

namespace MyosotisFW::System::Render
{
	LightingPipeline::~LightingPipeline()
	{
		vkDestroyPipeline(*m_device, m_pipeline, m_device->GetAllocationCallbacks());
		vkDestroyPipelineLayout(*m_device, m_pipelineLayout, m_device->GetAllocationCallbacks());

		vmaDestroyBuffer(m_device->GetVmaAllocator(), m_vertexBuffer.buffer, m_vertexBuffer.allocation);
		vmaDestroyBuffer(m_device->GetVmaAllocator(), m_indexBuffer.buffer, m_indexBuffer.allocation);
	}

	void LightingPipeline::Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass)
	{
		prepareRenderPipeline(resources, renderPass);

		for (uint32_t i = 0; i < AppInfo::g_maxInFlightFrameCount; i++)
		{
			{// Visibility Buffer
				Image visibilityBuffer = resources->GetVisibilityBuffer(i);
				// Sampler
				visibilityBuffer.sampler = resources->CreateSampler(Utility::Vulkan::CreateInfo::samplerCreateInfo());
				// descriptorImageInfo
				VkDescriptorImageInfo descriptorImageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(
					visibilityBuffer.sampler, visibilityBuffer.view,
					VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				// add to descriptor set
				pushConstant[i].visibilityBufferTextureID = m_textureDescriptorSet->AddImage(TextureDescriptorSet::DescriptorBindingIndex::CombinedImageSampler, descriptorImageInfo);
			}
		}
	}

	void LightingPipeline::BindCommandBuffer(const VkCommandBuffer& commandBuffer, const uint32_t frameIndex)
	{
		vkCmdBindPipeline(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
		std::vector<VkDescriptorSet> descriptorSets = m_renderDescriptors->GetDescriptorSet();
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0,
			static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, NULL);
		vkCmdPushConstants(commandBuffer, m_pipelineLayout,
			VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			static_cast<uint32_t>(sizeof(PushConstant)), &pushConstant[frameIndex]);
		vkCmdDraw(commandBuffer, 3, 1, 0, 0);
	}

	void LightingPipeline::Resize(const RenderResources_ptr& resources)
	{
		for (uint32_t i = 0; i < AppInfo::g_maxInFlightFrameCount; i++)
		{
			{// Visibility Buffer
				Image visibilityBuffer = resources->GetVisibilityBuffer(i);
				// Sampler
				visibilityBuffer.sampler = resources->CreateSampler(Utility::Vulkan::CreateInfo::samplerCreateInfo());
				// descriptorImageInfo
				VkDescriptorImageInfo descriptorImageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(
					visibilityBuffer.sampler, visibilityBuffer.view,
					VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				// Update
				m_textureDescriptorSet->UpdateImage(pushConstant[i].visibilityBufferTextureID, TextureDescriptorSet::DescriptorBindingIndex::CombinedImageSampler, descriptorImageInfo);
			}
		}
	}

	void LightingPipeline::prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass)
	{
		// push constant
		std::vector<VkPushConstantRange> pushConstantRange = {
			// PS
			Utility::Vulkan::CreateInfo::pushConstantRange(VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				static_cast<uint32_t>(sizeof(PushConstant))),
		};

		// [pipeline]layout
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts = m_renderDescriptors->GetDescriptorSetLayout();
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = Utility::Vulkan::CreateInfo::pipelineLayoutCreateInfo(descriptorSetLayouts);
		pipelineLayoutCreateInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRange.size());
		pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRange.data();
		VK_VALIDATION(vkCreatePipelineLayout(*m_device, &pipelineLayoutCreateInfo, m_device->GetAllocationCallbacks(), &m_pipelineLayout));

		// pipeline
		std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo{
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT, resources->GetShaderModules("Lighting.vert.spv")),
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT, resources->GetShaderModules("Lighting.frag.spv")),
		};

		VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineVertexInputStateCreateInfo();
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineInputAssemblyStateCreateInfo(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		VkPipelineViewportStateCreateInfo viewportStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineViewportStateCreateInfo();
		VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineRasterizationStateCreateInfo(VkPolygonMode::VK_POLYGON_MODE_FILL, VkCullModeFlagBits::VK_CULL_MODE_NONE, VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
		VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineMultisampleStateCreateInfo();
		VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VkCompareOp::VK_COMPARE_OP_LESS_OR_EQUAL);
		std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentStates = {
			Utility::Vulkan::CreateInfo::pipelineColorBlendAttachmentState(VK_FALSE),
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