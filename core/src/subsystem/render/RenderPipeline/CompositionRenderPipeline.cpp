// Copyright (c) 2025 kong9812
#include "CompositionRenderPipeline.h"
#include "VK_CreateInfo.h"
#include "appInfo.h"

namespace MyosotisFW::System::Render
{
	CompositionRenderPipeline::CompositionRenderPipeline(RenderDevice_ptr device, RenderResources_ptr resources, VkRenderPass renderPass)
	{
		m_device = device;
		m_descriptorCount = AppInfo::g_descriptorCount;
		prepareDescriptors();
		prepareRenderPipeline(resources, renderPass);
	}

	CompositionRenderPipeline::~CompositionRenderPipeline()
	{
		vkDestroyDescriptorSetLayout(*m_device, m_descriptorSetLayout, m_device->GetAllocationCallbacks());
		vkDestroyDescriptorPool(*m_device, m_descriptorPool, m_device->GetAllocationCallbacks());
		vkDestroyPipeline(*m_device, m_pipeline, m_device->GetAllocationCallbacks());
		vkDestroyPipelineLayout(*m_device, m_pipelineLayout, m_device->GetAllocationCallbacks());
	}
	void CompositionRenderPipeline::CreateShaderObject(ShaderBase& shaderBase, VMAImage position, VMAImage baseColor)
	{
		{// pipeline
			shaderBase.pipelineLayout = m_pipelineLayout;
			shaderBase.pipeline = m_pipeline;
		}

		// layout allocate
		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = Utility::Vulkan::CreateInfo::descriptorSetAllocateInfo(m_descriptorPool, &m_descriptorSetLayout);
		VK_VALIDATION(vkAllocateDescriptorSets(*m_device, &descriptorSetAllocateInfo, &shaderBase.descriptorSet));
		
		VkDescriptorImageInfo positionDescriptorImageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(VK_NULL_HANDLE, position.view, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		VkDescriptorImageInfo baseColorDescriptorImageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(VK_NULL_HANDLE, baseColor.view, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		// write descriptor set
		std::vector<VkWriteDescriptorSet> writeDescriptorSet = {
			Utility::Vulkan::CreateInfo::writeDescriptorSet(shaderBase.descriptorSet, 0, VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, &positionDescriptorImageInfo),
			Utility::Vulkan::CreateInfo::writeDescriptorSet(shaderBase.descriptorSet, 1, VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, &baseColorDescriptorImageInfo),
		};
		vkUpdateDescriptorSets(*m_device, static_cast<uint32_t>(writeDescriptorSet.size()), writeDescriptorSet.data(), 0, nullptr);
	}

	void CompositionRenderPipeline::prepareDescriptors()
	{
		std::vector<VkDescriptorPoolSize> poolSize = {
			Utility::Vulkan::CreateInfo::descriptorPoolSize(VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 2)
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

	void CompositionRenderPipeline::prepareRenderPipeline(RenderResources_ptr resources, VkRenderPass renderPass)
	{
		// [pipeline]layout
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = Utility::Vulkan::CreateInfo::pipelineLayoutCreateInfo(&m_descriptorSetLayout);
		VK_VALIDATION(vkCreatePipelineLayout(*m_device, &pipelineLayoutCreateInfo, m_device->GetAllocationCallbacks(), &m_pipelineLayout));

		// pipeline
		std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo{
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT, resources->GetShaderModules("StaticMesh_Composition.vert.spv")),
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT, resources->GetShaderModules("StaticMesh_Composition.frag.spv")),
		};

		// pipelineVertexInputStateCreateInfo
		std::vector<VkVertexInputBindingDescription> vertexInputBindingDescription = {
			Utility::Vulkan::CreateInfo::vertexInputBindingDescription(0, Utility::Vulkan::CreateInfo::VertexAttributeBit::POSITION_VEC4 | Utility::Vulkan::CreateInfo::VertexAttributeBit::COLOR_VEC4)
		};
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptiones = Utility::Vulkan::CreateInfo::vertexInputAttributeDescriptiones(0,
			Utility::Vulkan::CreateInfo::VertexAttributeBit::POSITION_VEC4 | Utility::Vulkan::CreateInfo::VertexAttributeBit::COLOR_VEC4);
		VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineVertexInputStateCreateInfo(vertexInputBindingDescription, vertexInputAttributeDescriptiones);

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
		graphicsPipelineCreateInfo.subpass = 1;
		VK_VALIDATION(vkCreateGraphicsPipelines(*m_device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, m_device->GetAllocationCallbacks(), &m_pipeline));
	}

}