// Copyright (c) 2025 kong9812
#include "ShadowMapRenderPipeline.h"
#include "VK_CreateInfo.h"
#include "AppInfo.h"

namespace {
	// todo. 外部から取って来れるように変更
	constexpr glm::vec3 g_lightPos = glm::vec3(-20.0f, 40.0f, -20.0f);
	constexpr glm::vec3 g_lightUp = glm::vec3(0.0f, -1.0f, 0.0f);
	constexpr float g_lightFOV = 60.0f;
	constexpr float g_lightNear = 20.0f;
	constexpr float g_lightFar = 100.0f;
	constexpr float g_aspectRadio = 1.0f;
	constexpr glm::vec3 g_lightLookAt = glm::vec3(0.0f);
	constexpr uint32_t g_pcfCount = 2;

	inline glm::mat4 GetLightViewProject()
	{
		glm::mat4 projection = glm::perspective(glm::radians(g_lightFOV), g_aspectRadio, g_lightNear, g_lightFar);
		glm::mat4 view = glm::lookAt(g_lightPos, g_lightLookAt, g_lightUp);
		return projection * view;
	}
}

namespace MyosotisFW::System::Render
{
	ShadowMapRenderPipeline::~ShadowMapRenderPipeline()
	{
		vkDestroySampler(*m_device, m_shadowMapSampler, m_device->GetAllocationCallbacks());
		vkDestroyPipeline(*m_device, m_pipeline, m_device->GetAllocationCallbacks());
		vkDestroyPipelineLayout(*m_device, m_pipelineLayout, m_device->GetAllocationCallbacks());
	}

	void ShadowMapRenderPipeline::Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass)
	{
		prepareRenderPipeline(resources, renderPass);

		VkSamplerCreateInfo samplerCreateInfo = Utility::Vulkan::CreateInfo::samplerCreateInfo();
		VK_VALIDATION(vkCreateSampler(*m_device, &samplerCreateInfo, m_device->GetAllocationCallbacks(), &m_shadowMapSampler));
		m_shadowMapDescriptorImageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(m_shadowMapSampler, resources->GetShadowMap().view, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void ShadowMapRenderPipeline::CreateShaderObject(ShadowMapShaderObject& shaderObject)
	{
		{// pipeline
			shaderObject.shaderBase.pipelineLayout = m_pipelineLayout;
			shaderObject.shaderBase.pipeline = m_pipeline;
		}

		// layout allocate
		shaderObject.shaderBase.descriptorSet = m_descriptors->GetBindlessDescriptorSet();
	}

	void ShadowMapRenderPipeline::UpdateDescriptors(ShadowMapShaderObject& shaderObject)
	{
		shaderObject.pushConstant.objectIndex = m_descriptors->AddStorageBuffer(shaderObject.SSBO);
		shaderObject.pushConstant.textureId = m_descriptors->AddCombinedImageSamplerInfo(m_shadowMapDescriptorImageInfo);
	}

	DirectionalLightSSBO ShadowMapRenderPipeline::GetDirectionalLightInfo()
	{
		DirectionalLightSSBO lightInfo{};
		lightInfo.viewProjection = GetLightViewProject();
		lightInfo.position = glm::vec4(g_lightPos, 1.0f);
		lightInfo.pcfCount = g_pcfCount;
		return lightInfo;
	}

	void ShadowMapRenderPipeline::prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass)
	{
		// push constant
		std::vector<VkPushConstantRange> pushConstantRange = {
			// VS
			Utility::Vulkan::CreateInfo::pushConstantRange(VkShaderStageFlagBits::VK_SHADER_STAGE_ALL,
				0,
				static_cast<uint32_t>(sizeof(StaticMeshShaderObject::pushConstant))),
		};

		// [pipeline]layout
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts = { m_descriptors->GetBindlessDescriptorSetLayout() };
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = Utility::Vulkan::CreateInfo::pipelineLayoutCreateInfo(descriptorSetLayouts);
		pipelineLayoutCreateInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRange.size());
		pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRange.data();
		VK_VALIDATION(vkCreatePipelineLayout(*m_device, &pipelineLayoutCreateInfo, m_device->GetAllocationCallbacks(), &m_pipelineLayout));

		// pipeline
		std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo{
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT, resources->GetShaderModules("ShadowMap.vert.spv")),
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
		VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineColorBlendStateCreateInfo();
		std::vector<VkDynamicState> dynamicStates = { VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT, VkDynamicState::VK_DYNAMIC_STATE_SCISSOR, VkDynamicState::VK_DYNAMIC_STATE_DEPTH_BIAS };
		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineDynamicStateCreateInfo(dynamicStates);
		rasterizationStateCreateInfo.depthBiasEnable = VK_TRUE;

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