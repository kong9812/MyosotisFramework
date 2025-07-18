// Copyright (c) 2025 kong9812
#include "EditorFinalCompositionRenderPipeline.h"
#include "EditorRenderResources.h"
#include "VK_CreateInfo.h"
#include "AppInfo.h"

namespace MyosotisFW::System::Render
{
	void EditorFinalCompositionRenderPipeline::Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass)
	{
		prepareDescriptors();
		prepareRenderPipeline(resources, renderPass);

		m_mainRenderTargetDescriptorImageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(VK_NULL_HANDLE, resources->GetMainRenderTarget().view, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		m_editorRenderTargetDescriptorImageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(VK_NULL_HANDLE, std::dynamic_pointer_cast<EditorRenderResources>(resources)->GetEditorRenderTarget().view, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void EditorFinalCompositionRenderPipeline::CreateShaderObject()
	{
		{// pipeline
			m_shaderBase.pipelineLayout = m_pipelineLayout;
			m_shaderBase.pipeline = m_pipeline;
		}

		// layout allocate
		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = Utility::Vulkan::CreateInfo::descriptorSetAllocateInfo(m_descriptorPool, &m_descriptorSetLayout);
		VK_VALIDATION(vkAllocateDescriptorSets(*m_device, &descriptorSetAllocateInfo, &m_shaderBase.descriptorSet));

		// write descriptor set
		UpdateDescriptors();
	}

	void EditorFinalCompositionRenderPipeline::UpdateDescriptors()
	{
		std::vector<VkWriteDescriptorSet> writeDescriptorSet = {
			Utility::Vulkan::CreateInfo::writeDescriptorSet(m_shaderBase.descriptorSet, 0, VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, &m_mainRenderTargetDescriptorImageInfo),
			Utility::Vulkan::CreateInfo::writeDescriptorSet(m_shaderBase.descriptorSet, 1, VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, &m_editorRenderTargetDescriptorImageInfo),
		};
		vkUpdateDescriptorSets(*m_device, static_cast<uint32_t>(writeDescriptorSet.size()), writeDescriptorSet.data(), 0, nullptr);
	}

	void EditorFinalCompositionRenderPipeline::Resize(const RenderResources_ptr& resources)
	{
		m_mainRenderTargetDescriptorImageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(VK_NULL_HANDLE, resources->GetMainRenderTarget().view, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		m_editorRenderTargetDescriptorImageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(VK_NULL_HANDLE, std::dynamic_pointer_cast<EditorRenderResources>(resources)->GetEditorRenderTarget().view, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		UpdateDescriptors();
	}

	void EditorFinalCompositionRenderPipeline::prepareDescriptors()
	{
		std::vector<VkDescriptorPoolSize> poolSize = {
			Utility::Vulkan::CreateInfo::descriptorPoolSize(VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 2),
		};
		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = Utility::Vulkan::CreateInfo::descriptorPoolCreateInfo(poolSize, m_descriptorCount);
		VK_VALIDATION(vkCreateDescriptorPool(*m_device, &descriptorPoolCreateInfo, m_device->GetAllocationCallbacks(), &m_descriptorPool));

		// [descriptor]layout
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBinding = {
			// binding: 0
			Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(0, VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT),
			// binding: 1
			Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(1, VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT),
		};
		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = Utility::Vulkan::CreateInfo::descriptorSetLayoutCreateInfo(setLayoutBinding);
		VK_VALIDATION(vkCreateDescriptorSetLayout(*m_device, &descriptorSetLayoutCreateInfo, m_device->GetAllocationCallbacks(), &m_descriptorSetLayout));
	}

	void EditorFinalCompositionRenderPipeline::prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass)
	{
		// [pipeline]layout
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = Utility::Vulkan::CreateInfo::pipelineLayoutCreateInfo(&m_descriptorSetLayout);
		VK_VALIDATION(vkCreatePipelineLayout(*m_device, &pipelineLayoutCreateInfo, m_device->GetAllocationCallbacks(), &m_pipelineLayout));

		// pipeline
		std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo{
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT, resources->GetShaderModules("EditorFinalComposition.vert.spv")),
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT, resources->GetShaderModules("EditorFinalComposition.frag.spv")),
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