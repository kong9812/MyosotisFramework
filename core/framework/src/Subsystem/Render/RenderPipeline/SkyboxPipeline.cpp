// Copyright (c) 2025 kong9812
#include "SkyboxPipeline.h"
#include "VK_CreateInfo.h"
#include "AppInfo.h"

namespace MyosotisFW::System::Render
{
	SkyboxPipeline::~SkyboxPipeline()
	{
		vkDestroyPipeline(*m_device, m_pipeline, m_device->GetAllocationCallbacks());
		vkDestroyPipelineLayout(*m_device, m_pipelineLayout, m_device->GetAllocationCallbacks());
	}

	void SkyboxPipeline::Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass)
	{
		prepareRenderPipeline(resources, renderPass);

		{// Skybox Mesh
			MeshHandle meshHandle = resources->GetPrimitiveGeometryMesh(Shape::PrimitiveGeometryShape::Quad);
			std::shared_ptr<const Mesh> mesh = meshHandle.lock();
			m_vertexBuffer = mesh->vertexBuffer;
			m_indexBuffer = mesh->indexBuffer;
		}

		{// Skybox Image
			Image skyboxImage = resources->GetCubeImage({
				"sky\\right.png",
				"sky\\left.png",
				"sky\\top.png",
				"sky\\bottom.png",
				"sky\\front.png",
				"sky\\back.png" });
			// sampler
			skyboxImage.sampler = resources->CreateSampler(Utility::Vulkan::CreateInfo::samplerCreateInfo());
			// descriptorImageInfo
			VkDescriptorImageInfo descriptorImageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(
				skyboxImage.sampler, skyboxImage.view,
				VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			// add to descriptor set
			pushConstant.skyboxTextureID = m_textureDescriptorSet->AddImage(TextureDescriptorSet::DescriptorBindingIndex::CombinedImageSampler, descriptorImageInfo);
		}
	}

	void SkyboxPipeline::BindCommandBuffer(const VkCommandBuffer& commandBuffer)
	{
		vkCmdBindPipeline(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
		std::vector<VkDescriptorSet> descriptorSets = m_renderDescriptors->GetDescriptorSet();
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0,
			static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, NULL);
		vkCmdPushConstants(commandBuffer, m_pipelineLayout,
			VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			static_cast<uint32_t>(sizeof(pushConstant)), &pushConstant);
		const VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuffer.buffer, offsets);
		vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer.buffer, 0, VkIndexType::VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(commandBuffer, m_indexBuffer.localSize / sizeof(uint32_t), 1, 0, 0, 0);
	}

	void SkyboxPipeline::prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass)
	{
		// push constant
		std::vector<VkPushConstantRange> pushConstantRange = {
			// PS
			Utility::Vulkan::CreateInfo::pushConstantRange(VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				static_cast<uint32_t>(sizeof(pushConstant))),
		};

		// [pipeline]layout
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts = m_renderDescriptors->GetDescriptorSetLayout();
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = Utility::Vulkan::CreateInfo::pipelineLayoutCreateInfo(descriptorSetLayouts);
		pipelineLayoutCreateInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRange.size());
		pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRange.data();
		VK_VALIDATION(vkCreatePipelineLayout(*m_device, &pipelineLayoutCreateInfo, m_device->GetAllocationCallbacks(), &m_pipelineLayout));

		// pipeline
		std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo{
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT, resources->GetShaderModules("Skybox.vert.spv")),
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT, resources->GetShaderModules("Skybox.frag.spv")),
		};

		// pipelineVertexInputStateCreateInfo
		Utility::Vulkan::CreateInfo::VertexAttributeBits vertexAttributeBits =
			Utility::Vulkan::CreateInfo::VertexAttributeBit::POSITION_VEC3 |
			Utility::Vulkan::CreateInfo::VertexAttributeBit::NORMAL |
			Utility::Vulkan::CreateInfo::VertexAttributeBit::UV0 |
			Utility::Vulkan::CreateInfo::VertexAttributeBit::UV1 |
			Utility::Vulkan::CreateInfo::VertexAttributeBit::COLOR_VEC4;

		// pipelineVertexInputStateCreateInfo
		std::vector<VkVertexInputBindingDescription> vertexInputBindingDescription = {
			Utility::Vulkan::CreateInfo::vertexInputBindingDescription(0, vertexAttributeBits)
		};
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptiones = Utility::Vulkan::CreateInfo::vertexInputAttributeDescriptiones(0, vertexAttributeBits);
		VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineVertexInputStateCreateInfo(vertexInputBindingDescription, vertexInputAttributeDescriptiones);

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineInputAssemblyStateCreateInfo(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		VkPipelineViewportStateCreateInfo viewportStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineViewportStateCreateInfo();
		VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineRasterizationStateCreateInfo(VkPolygonMode::VK_POLYGON_MODE_FILL, VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT, VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
		VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineMultisampleStateCreateInfo();
		VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VkCompareOp::VK_COMPARE_OP_NEVER);
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