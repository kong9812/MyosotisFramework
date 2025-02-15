// Copyright (c) 2025 kong9812
#include "ShadowMapRenderPipeline.h"
#include "VK_CreateInfo.h"
#include "AppInfo.h"

namespace {
	// todo. 外部から取って来れるように変更
	constexpr glm::vec3 g_lightPos = glm::vec3(24.5f, 33.1f, -34.2f);
	constexpr glm::vec3 g_lightUp = glm::vec3(0.0f, -1.0f, 0.0f);
	constexpr float g_lightFOV = 60.0f;
	constexpr float g_lightFar = 1000.0f;
	constexpr float g_lightNear = 1.0f;
	constexpr float g_aspectRadio = 1.7f;
	constexpr glm::vec3 g_lightLookAt = glm::vec3(0.0f);

	inline glm::mat4 GetLightViewProject()
	{
		glm::mat4 view = glm::lookAt(g_lightPos, g_lightLookAt, g_lightUp);
		glm::mat4 projection = glm::perspective(glm::radians(g_lightFOV), g_aspectRadio, g_lightNear, g_lightFar);
		return projection * view;
	}
}

namespace MyosotisFW::System::Render
{
	ShadowMapRenderPipeline::ShadowMapRenderPipeline(RenderDevice_ptr device, RenderResources_ptr resources, VkRenderPass renderPass)
	{
		m_device = device;
		m_descriptorCount = AppInfo::g_descriptorCount;
		prepareDescriptors();
		prepareRenderPipeline(resources, renderPass);

		m_shadowMapShaderObject.lightUBO.data.lightViewProjection = GetLightViewProject();
		vmaTools::ShaderBufferObjectAllocate(
			*m_device,
			m_device->GetVmaAllocator(),
			m_shadowMapShaderObject.lightUBO.data,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			m_shadowMapShaderObject.lightUBO.buffer.buffer,
			m_shadowMapShaderObject.lightUBO.buffer.allocation,
			m_shadowMapShaderObject.lightUBO.buffer.allocationInfo,
			m_shadowMapShaderObject.lightUBO.buffer.descriptor);
		memcpy(m_shadowMapShaderObject.lightUBO.buffer.allocationInfo.pMappedData, &m_shadowMapShaderObject.lightUBO.data, sizeof(m_shadowMapShaderObject.lightUBO.data));
	}

	ShadowMapRenderPipeline::~ShadowMapRenderPipeline()
	{
		vmaDestroyBuffer(m_device->GetVmaAllocator(), m_shadowMapShaderObject.lightUBO.buffer.buffer, m_shadowMapShaderObject.lightUBO.buffer.allocation);
		vkDestroyDescriptorSetLayout(*m_device, m_descriptorSetLayout, m_device->GetAllocationCallbacks());
		vkDestroyDescriptorPool(*m_device, m_descriptorPool, m_device->GetAllocationCallbacks());
		vkDestroyPipeline(*m_device, m_pipeline, m_device->GetAllocationCallbacks());
		vkDestroyPipelineLayout(*m_device, m_pipelineLayout, m_device->GetAllocationCallbacks());
	}

	void ShadowMapRenderPipeline::CreateShaderObject(StaticMeshShaderObject& shaderObject)
	{
		{// pipeline
			shaderObject.shadowMapRenderShaderBase.pipelineLayout = m_pipelineLayout;
			shaderObject.shadowMapRenderShaderBase.pipeline = m_pipeline;
		}

		// layout allocate
		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = Utility::Vulkan::CreateInfo::descriptorSetAllocateInfo(m_descriptorPool, &m_descriptorSetLayout);
		VK_VALIDATION(vkAllocateDescriptorSets(*m_device, &descriptorSetAllocateInfo, &shaderObject.shadowMapRenderShaderBase.descriptorSet));

		VkDescriptorImageInfo descriptorImageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(shaderObject.standardUBO.normalMap.sampler, shaderObject.standardUBO.normalMap.image.view, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		// write descriptor set
		std::vector<VkWriteDescriptorSet> writeDescriptorSet = {
			Utility::Vulkan::CreateInfo::writeDescriptorSet(shaderObject.shadowMapRenderShaderBase.descriptorSet, 0, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,&m_shadowMapShaderObject.lightUBO.buffer.descriptor),
			Utility::Vulkan::CreateInfo::writeDescriptorSet(shaderObject.shadowMapRenderShaderBase.descriptorSet, 1, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &shaderObject.standardUBO.buffer.descriptor),
		};
		vkUpdateDescriptorSets(*m_device, static_cast<uint32_t>(writeDescriptorSet.size()), writeDescriptorSet.data(), 0, nullptr);
	}

	void ShadowMapRenderPipeline::prepareDescriptors()
	{
		std::vector<VkDescriptorPoolSize> poolSize = {
			Utility::Vulkan::CreateInfo::descriptorPoolSize(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2),
		};
		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = Utility::Vulkan::CreateInfo::descriptorPoolCreateInfo(poolSize, m_descriptorCount);
		VK_VALIDATION(vkCreateDescriptorPool(*m_device, &descriptorPoolCreateInfo, m_device->GetAllocationCallbacks(), &m_descriptorPool));

		// [descriptor]layout
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBinding = {
			// binding: 0
			Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(0, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT),
			// binding: 1
			Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(1, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT),
		};
		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = Utility::Vulkan::CreateInfo::descriptorSetLayoutCreateInfo(setLayoutBinding);
		VK_VALIDATION(vkCreateDescriptorSetLayout(*m_device, &descriptorSetLayoutCreateInfo, m_device->GetAllocationCallbacks(), &m_descriptorSetLayout));
	}

	void ShadowMapRenderPipeline::prepareRenderPipeline(RenderResources_ptr resources, VkRenderPass renderPass)
	{
		// [pipeline]layout
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = Utility::Vulkan::CreateInfo::pipelineLayoutCreateInfo(&m_descriptorSetLayout);
		VK_VALIDATION(vkCreatePipelineLayout(*m_device, &pipelineLayoutCreateInfo, m_device->GetAllocationCallbacks(), &m_pipelineLayout));

		// pipeline
		std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo{
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT, resources->GetShaderModules("ShadowMap.vert.spv")),
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
		VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineColorBlendStateCreateInfo();
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
		VK_VALIDATION(vkCreateGraphicsPipelines(*m_device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, m_device->GetAllocationCallbacks(), &m_pipeline));
	}
}