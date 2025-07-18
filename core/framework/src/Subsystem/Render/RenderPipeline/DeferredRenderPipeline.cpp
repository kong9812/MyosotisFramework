// Copyright (c) 2025 kong9812
#include "DeferredRenderPipeline.h"
#include "VK_CreateInfo.h"
#include "AppInfo.h"

namespace MyosotisFW::System::Render
{
	DeferredRenderPipeline::~DeferredRenderPipeline()
	{
		vkDestroyDescriptorSetLayout(*m_device, m_descriptorSetLayout, m_device->GetAllocationCallbacks());
		vkDestroyDescriptorPool(*m_device, m_descriptorPool, m_device->GetAllocationCallbacks());
		vkDestroyPipeline(*m_device, m_pipeline, m_device->GetAllocationCallbacks());
		vkDestroyPipelineLayout(*m_device, m_pipelineLayout, m_device->GetAllocationCallbacks());
	}

	void DeferredRenderPipeline::Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass)
	{
		prepareDescriptors();
		prepareRenderPipeline(resources, renderPass);
	}

	void DeferredRenderPipeline::CreateShaderObject(StaticMeshShaderObject& shaderObject)
	{
		{// pipeline
			shaderObject.deferredRenderShaderBase.pipelineLayout = m_pipelineLayout;
			shaderObject.deferredRenderShaderBase.pipeline = m_pipeline;
			shaderObject.deferredRenderShaderBase.descriptorPool = m_descriptorPool;
		}

		// layout allocate
		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = Utility::Vulkan::CreateInfo::descriptorSetAllocateInfo(m_descriptorPool, &m_descriptorSetLayout);
		VK_VALIDATION(vkAllocateDescriptorSets(*m_device, &descriptorSetAllocateInfo, &shaderObject.deferredRenderShaderBase.descriptorSet));

		UpdateDescriptors(shaderObject);
	}

	void DeferredRenderPipeline::UpdateDescriptors(StaticMeshShaderObject& shaderObject)
	{
		std::vector<VkWriteDescriptorSet> writeDescriptorSet = {};
		VkDescriptorImageInfo descriptorImageInfo{};
		if (shaderObject.standardUBO.useNormalMap)
		{
			descriptorImageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(shaderObject.standardUBO.normalMap.sampler, shaderObject.standardUBO.normalMap.view, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			// write descriptor set
			writeDescriptorSet = {
				Utility::Vulkan::CreateInfo::writeDescriptorSet(shaderObject.deferredRenderShaderBase.descriptorSet, 0, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &shaderObject.standardUBO.buffer.descriptor),
				Utility::Vulkan::CreateInfo::writeDescriptorSet(shaderObject.deferredRenderShaderBase.descriptorSet, 1, VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &descriptorImageInfo),
				Utility::Vulkan::CreateInfo::writeDescriptorSet(shaderObject.deferredRenderShaderBase.descriptorSet, 2, VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &shaderObject.standardUBO.shadowMapImageInfo),
				Utility::Vulkan::CreateInfo::writeDescriptorSet(shaderObject.deferredRenderShaderBase.descriptorSet, 3, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &shaderObject.standardUBO.shadowMapBufferDescriptor),
			};
		}
		else
		{
			// write descriptor set
			writeDescriptorSet = {
				Utility::Vulkan::CreateInfo::writeDescriptorSet(shaderObject.deferredRenderShaderBase.descriptorSet, 0, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &shaderObject.standardUBO.buffer.descriptor),
				Utility::Vulkan::CreateInfo::writeDescriptorSet(shaderObject.deferredRenderShaderBase.descriptorSet, 2, VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &shaderObject.standardUBO.shadowMapImageInfo),
				Utility::Vulkan::CreateInfo::writeDescriptorSet(shaderObject.deferredRenderShaderBase.descriptorSet, 3, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &shaderObject.standardUBO.shadowMapBufferDescriptor),
			};
		}
		vkUpdateDescriptorSets(*m_device, static_cast<uint32_t>(writeDescriptorSet.size()), writeDescriptorSet.data(), 0, nullptr);
	}

	void DeferredRenderPipeline::prepareDescriptors()
	{
		std::vector<VkDescriptorPoolSize> poolSize = {
			Utility::Vulkan::CreateInfo::descriptorPoolSize(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2),
			Utility::Vulkan::CreateInfo::descriptorPoolSize(VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2)
		};
		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = Utility::Vulkan::CreateInfo::descriptorPoolCreateInfo(poolSize, m_descriptorCount);
		VK_VALIDATION(vkCreateDescriptorPool(*m_device, &descriptorPoolCreateInfo, m_device->GetAllocationCallbacks(), &m_descriptorPool));

		// [descriptor]layout
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBinding = {
			// binding: 0
			Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(0, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT),
			// binding: 1
			Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(1, VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT),
			// binding: 2
			Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(2, VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT),
			// binding: 3
			Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(3, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT),
		};
		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = Utility::Vulkan::CreateInfo::descriptorSetLayoutCreateInfo(setLayoutBinding);
		VK_VALIDATION(vkCreateDescriptorSetLayout(*m_device, &descriptorSetLayoutCreateInfo, m_device->GetAllocationCallbacks(), &m_descriptorSetLayout));
	}

	void DeferredRenderPipeline::prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass)
	{
		// [pipeline]layout
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = Utility::Vulkan::CreateInfo::pipelineLayoutCreateInfo(&m_descriptorSetLayout);
		VK_VALIDATION(vkCreatePipelineLayout(*m_device, &pipelineLayoutCreateInfo, m_device->GetAllocationCallbacks(), &m_pipelineLayout));

		// pipeline
		std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo{
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT, resources->GetShaderModules("StaticMesh_Deferred.vert.spv")),
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT, resources->GetShaderModules("StaticMesh_Deferred.frag.spv")),
		};

		Utility::Vulkan::CreateInfo::VertexAttributeBits vertexAttributeBits =
			Utility::Vulkan::CreateInfo::VertexAttributeBit::POSITION_VEC4 |
			Utility::Vulkan::CreateInfo::VertexAttributeBit::NORMAL |
			Utility::Vulkan::CreateInfo::VertexAttributeBit::UV |
			Utility::Vulkan::CreateInfo::VertexAttributeBit::COLOR_VEC4;

		// pipelineVertexInputStateCreateInfo
		std::vector<VkVertexInputBindingDescription> vertexInputBindingDescription = {
			Utility::Vulkan::CreateInfo::vertexInputBindingDescription(0, vertexAttributeBits)
		};
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptiones = Utility::Vulkan::CreateInfo::vertexInputAttributeDescriptiones(0, vertexAttributeBits);
		VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineVertexInputStateCreateInfo(vertexInputBindingDescription, vertexInputAttributeDescriptiones);

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineInputAssemblyStateCreateInfo(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		VkPipelineViewportStateCreateInfo viewportStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineViewportStateCreateInfo();
		VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineRasterizationStateCreateInfo(VkPolygonMode::VK_POLYGON_MODE_FILL, VkCullModeFlagBits::VK_CULL_MODE_FRONT_BIT, VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
		VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineMultisampleStateCreateInfo();
		VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VkCompareOp::VK_COMPARE_OP_LESS_OR_EQUAL);
		std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentStates = {
			Utility::Vulkan::CreateInfo::pipelineColorBlendAttachmentState(VK_FALSE),
			Utility::Vulkan::CreateInfo::pipelineColorBlendAttachmentState(VK_FALSE),
			Utility::Vulkan::CreateInfo::pipelineColorBlendAttachmentState(VK_FALSE),
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