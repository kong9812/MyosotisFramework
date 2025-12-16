// Copyright (c) 2025 kong9812
#include "VisibilityBufferRenderPhase1Pipeline.h"
#include "VK_CreateInfo.h"
#include "AppInfo.h"

#include "PrimitiveGeometryShape.h"

namespace MyosotisFW::System::Render
{
	VisibilityBufferRenderPhase1Pipeline::~VisibilityBufferRenderPhase1Pipeline()
	{
		vkDestroyPipeline(*m_device, m_pipeline, m_device->GetAllocationCallbacks());
		vkDestroyPipelineLayout(*m_device, m_pipelineLayout, m_device->GetAllocationCallbacks());
	}

	void VisibilityBufferRenderPhase1Pipeline::Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass)
	{
		m_vkCmdDrawMeshTasksEXT = (PFN_vkCmdDrawMeshTasksEXT)vkGetDeviceProcAddr(*m_device, "vkCmdDrawMeshTasksEXT");

		prepareRenderPipeline(resources, renderPass);

		VkDescriptorImageInfo descriptorImageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(
			resources->GetHiZDepthMap().sampler, resources->GetHiZDepthMap().view,
			VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		m_hiZSamplerID = m_textureDescriptorSet->AddImage(TextureDescriptorSet::DescriptorBindingIndex::CombinedImageSampler, descriptorImageInfo);

		descriptorImageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(
			resources->GetPrimaryDepthStencil().sampler, resources->GetPrimaryDepthStencil().view,
			VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		m_primaryDepthSamplerID = m_textureDescriptorSet->AddImage(TextureDescriptorSet::DescriptorBindingIndex::CombinedImageSampler, descriptorImageInfo);

		pushConstant.hiZMipLevelMax = static_cast<float>(resources->GetHiZDepthMap().mipView.size()) - 1.0f;
	}

	void VisibilityBufferRenderPhase1Pipeline::BindCommandBuffer(const VkCommandBuffer& commandBuffer, const uint32_t vbDispatchInfoCount)
	{
		{// Phase1Render
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
			std::vector<VkDescriptorSet> descriptorSets = m_renderDescriptors->GetDescriptorSet();
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0,
				static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, NULL);
			pushConstant.hiZSamplerID = m_hiZSamplerID;
			pushConstant.vbDispatchInfoCount = vbDispatchInfoCount;
			vkCmdPushConstants(commandBuffer, m_pipelineLayout,
				VkShaderStageFlagBits::VK_SHADER_STAGE_TASK_BIT_EXT,
				0, static_cast<uint32_t>(sizeof(pushConstant)), &pushConstant);
			uint32_t taskGroupSize = static_cast<uint32_t>(ceil(static_cast<float>(vbDispatchInfoCount) / 128.0f));
			m_vkCmdDrawMeshTasksEXT(commandBuffer, taskGroupSize, 1, 1);
		}
	}

	void VisibilityBufferRenderPhase1Pipeline::prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass)
	{
		// push constant
		std::vector<VkPushConstantRange> pushConstantRange = {
			// VS
			Utility::Vulkan::CreateInfo::pushConstantRange(VkShaderStageFlagBits::VK_SHADER_STAGE_TASK_BIT_EXT,
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
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_TASK_BIT_EXT, resources->GetShaderModules("VisibilityBuffer.task.spv")),
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT, resources->GetShaderModules("VisibilityBuffer.mesh.spv")),
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT, resources->GetShaderModules("VisibilityBuffer.frag.spv")),
		};

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineInputAssemblyStateCreateInfo(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		VkPipelineViewportStateCreateInfo viewportStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineViewportStateCreateInfo();
		VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineRasterizationStateCreateInfo(VkPolygonMode::VK_POLYGON_MODE_FILL, VkCullModeFlagBits::VK_CULL_MODE_FRONT_BIT, VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
		VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineMultisampleStateCreateInfo();
		VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VkCompareOp::VK_COMPARE_OP_LESS_OR_EQUAL);
		std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentStates = {
			Utility::Vulkan::CreateInfo::pipelineColorBlendAttachmentState(VK_FALSE),
		};
		VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineColorBlendStateCreateInfo(colorBlendAttachmentStates);
		std::vector<VkDynamicState> dynamicStates = { VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT, VkDynamicState::VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineDynamicStateCreateInfo(dynamicStates);

		VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = Utility::Vulkan::CreateInfo::graphicsPipelineCreateInfo(
			shaderStageCreateInfo,									// シェーダーステージ
			VK_NULL_HANDLE,											// 頂点入力
			VK_NULL_HANDLE,											// 入力アセンブリ
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