// Copyright (c) 2025 kong9812
#include "CompositionRenderPipeline.h"
#include "VK_CreateInfo.h"
#include "AppInfo.h"

namespace MyosotisFW::System::Render
{
	CompositionRenderPipeline::CompositionRenderPipeline(const RenderDevice_ptr& device, const RenderResources_ptr& resources, const VkRenderPass& renderPass)
	{
		m_device = device;
		m_descriptorCount = AppInfo::g_descriptorCount;
		prepareDescriptors();
		prepareRenderPipeline(resources, renderPass);

		vmaTools::ShaderBufferObjectAllocate(
			*m_device,
			m_device->GetVmaAllocator(),
			m_compositionShaderObject.cameraUBO.data,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			m_compositionShaderObject.cameraUBO.buffer.buffer,
			m_compositionShaderObject.cameraUBO.buffer.allocation,
			m_compositionShaderObject.cameraUBO.buffer.allocationInfo,
			m_compositionShaderObject.cameraUBO.buffer.descriptor);
		vmaTools::ShaderBufferObjectAllocate(
			*m_device,
			m_device->GetVmaAllocator(),
			m_compositionShaderObject.lightUBO.data,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			m_compositionShaderObject.lightUBO.buffer.buffer,
			m_compositionShaderObject.lightUBO.buffer.allocation,
			m_compositionShaderObject.lightUBO.buffer.allocationInfo,
			m_compositionShaderObject.lightUBO.buffer.descriptor);
	}

	CompositionRenderPipeline::~CompositionRenderPipeline()
	{
		vmaDestroyBuffer(m_device->GetVmaAllocator(), m_compositionShaderObject.cameraUBO.buffer.buffer, m_compositionShaderObject.cameraUBO.buffer.allocation);
		vmaDestroyBuffer(m_device->GetVmaAllocator(), m_compositionShaderObject.lightUBO.buffer.buffer, m_compositionShaderObject.lightUBO.buffer.allocation);
		vkDestroyDescriptorSetLayout(*m_device, m_descriptorSetLayout, m_device->GetAllocationCallbacks());
		vkDestroyDescriptorPool(*m_device, m_descriptorPool, m_device->GetAllocationCallbacks());
		vkDestroyPipeline(*m_device, m_pipeline, m_device->GetAllocationCallbacks());
		vkDestroyPipelineLayout(*m_device, m_pipelineLayout, m_device->GetAllocationCallbacks());
	}

	void CompositionRenderPipeline::BindCommandBuffer(const VkCommandBuffer& commandBuffer)
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_compositionShaderObject.shaderBase.pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_compositionShaderObject.shaderBase.pipelineLayout, 0, 1, &m_compositionShaderObject.shaderBase.descriptorSet, 0, NULL);
		vkCmdDraw(commandBuffer, 3, 1, 0, 0);
	}

	void CompositionRenderPipeline::UpdateDirectionalLightInfo(const DirectionalLightInfo& lightInfo)
	{
		m_compositionShaderObject.lightUBO.data = lightInfo;
		memcpy(m_compositionShaderObject.lightUBO.buffer.allocationInfo.pMappedData, &m_compositionShaderObject.lightUBO.data, sizeof(m_compositionShaderObject.lightUBO.data));
	}

	void CompositionRenderPipeline::UpdateCameraPosition(const glm::vec4& position)
	{
		m_compositionShaderObject.cameraUBO.data.position = position;
		memcpy(m_compositionShaderObject.cameraUBO.buffer.allocationInfo.pMappedData, &m_compositionShaderObject.cameraUBO.data, sizeof(m_compositionShaderObject.cameraUBO.data));
	}

	void CompositionRenderPipeline::CreateShaderObject(const VMAImage& position, const VMAImage& normal, const VMAImage& baseColor, const VkDescriptorImageInfo& shadowMapImageInfo)
	{
		{// pipeline
			m_compositionShaderObject.shaderBase.pipelineLayout = m_pipelineLayout;
			m_compositionShaderObject.shaderBase.pipeline = m_pipeline;
		}

		// layout allocate
		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = Utility::Vulkan::CreateInfo::descriptorSetAllocateInfo(m_descriptorPool, &m_descriptorSetLayout);
		VK_VALIDATION(vkAllocateDescriptorSets(*m_device, &descriptorSetAllocateInfo, &m_compositionShaderObject.shaderBase.descriptorSet));

		VkDescriptorImageInfo positionDescriptorImageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(VK_NULL_HANDLE, position.view, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		VkDescriptorImageInfo normalDescriptorImageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(VK_NULL_HANDLE, normal.view, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		VkDescriptorImageInfo baseColorDescriptorImageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(VK_NULL_HANDLE, baseColor.view, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		// write descriptor set
		std::vector<VkWriteDescriptorSet> writeDescriptorSet = {
			Utility::Vulkan::CreateInfo::writeDescriptorSet(m_compositionShaderObject.shaderBase.descriptorSet, 0, VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, &positionDescriptorImageInfo),
			Utility::Vulkan::CreateInfo::writeDescriptorSet(m_compositionShaderObject.shaderBase.descriptorSet, 1, VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, &normalDescriptorImageInfo),
			Utility::Vulkan::CreateInfo::writeDescriptorSet(m_compositionShaderObject.shaderBase.descriptorSet, 2, VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, &baseColorDescriptorImageInfo),
			Utility::Vulkan::CreateInfo::writeDescriptorSet(m_compositionShaderObject.shaderBase.descriptorSet, 3, VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &shadowMapImageInfo),
			Utility::Vulkan::CreateInfo::writeDescriptorSet(m_compositionShaderObject.shaderBase.descriptorSet, 4, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &m_compositionShaderObject.cameraUBO.buffer.descriptor),
			Utility::Vulkan::CreateInfo::writeDescriptorSet(m_compositionShaderObject.shaderBase.descriptorSet, 5, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &m_compositionShaderObject.lightUBO.buffer.descriptor),
		};
		vkUpdateDescriptorSets(*m_device, static_cast<uint32_t>(writeDescriptorSet.size()), writeDescriptorSet.data(), 0, nullptr);
	}

	void CompositionRenderPipeline::prepareDescriptors()
	{
		std::vector<VkDescriptorPoolSize> poolSize = {
			Utility::Vulkan::CreateInfo::descriptorPoolSize(VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 3),
			Utility::Vulkan::CreateInfo::descriptorPoolSize(VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1),
			Utility::Vulkan::CreateInfo::descriptorPoolSize(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2),
		};
		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = Utility::Vulkan::CreateInfo::descriptorPoolCreateInfo(poolSize, m_descriptorCount);
		VK_VALIDATION(vkCreateDescriptorPool(*m_device, &descriptorPoolCreateInfo, m_device->GetAllocationCallbacks(), &m_descriptorPool));

		// [descriptor]layout
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBinding = {
			// binding: 0
			Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(0, VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT),
			// binding: 1
			Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(1, VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT),
			// binding: 2
			Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(2, VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT),
			// binding: 3
			Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(3, VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT),
			// binding: 4
			Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(4, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT),
			// binding: 5
			Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(5, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT),
		};
		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = Utility::Vulkan::CreateInfo::descriptorSetLayoutCreateInfo(setLayoutBinding);
		VK_VALIDATION(vkCreateDescriptorSetLayout(*m_device, &descriptorSetLayoutCreateInfo, m_device->GetAllocationCallbacks(), &m_descriptorSetLayout));
	}

	void CompositionRenderPipeline::prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass)
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
		graphicsPipelineCreateInfo.subpass = 1;
		VK_VALIDATION(vkCreateGraphicsPipelines(*m_device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, m_device->GetAllocationCallbacks(), &m_pipeline));
	}
}