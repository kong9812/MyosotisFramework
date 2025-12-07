// Copyright (c) 2025 kong9812
#include "TerrainPipeline.h"
#include "VK_CreateInfo.h"
#include "AppInfo.h"
#include "TerrainIo.h"
#include "VBDispatchInfo.h"

#include "PrimitiveGeometryShape.h"

namespace MyosotisFW::System::Render
{
	TerrainPipeline::~TerrainPipeline()
	{
		vkDestroyPipeline(*m_device, m_pipeline, m_device->GetAllocationCallbacks());
		vkDestroyPipelineLayout(*m_device, m_pipelineLayout, m_device->GetAllocationCallbacks());
	}

	void TerrainPipeline::Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass)
	{
		m_vkCmdDrawMeshTasksEXT = (PFN_vkCmdDrawMeshTasksEXT)vkGetDeviceProcAddr(*m_device, "vkCmdDrawMeshTasksEXT");

		prepareRenderPipeline(resources, renderPass);

		std::vector<Mesh> mesh = Utility::Loader::loadTerrainMesh("terrain.png");
		std::vector<VBDispatchInfo> vbDispatchInfoList;
		for (uint32_t i = 0; i < mesh.size(); i++)
		{
			m_meshletSize += static_cast<uint32_t>(mesh[i].meshlet.size());
			std::vector<uint32_t> meshID = m_meshInfoDescriptorSet->AddCustomGeometry("terrain.png" + std::to_string(i), mesh);

			for (uint32_t j = 0; j < mesh[i].meshlet.size(); j++)
			{
				VBDispatchInfo vbDispatchInfo{};
				vbDispatchInfo.objectID = 0;		// 必要ない
				vbDispatchInfo.meshID = meshID[i];	// meshIDそのままを使って、iではない！
				vbDispatchInfo.meshletID = j;		// jでOK! GPUでmeshIDからmeshデータを取り出し、meshletOffsetを使って正しいIndexを取る
				vbDispatchInfoList.push_back(vbDispatchInfo);
			}
		}

		m_sceneInfoDescriptorSet->AddTerrainVBDispatchInfo(vbDispatchInfoList);

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

	void TerrainPipeline::BindCommandBuffer(const VkCommandBuffer& commandBuffer)
	{
		{// Phase1Render
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
			std::vector<VkDescriptorSet> descriptorSets = {
				m_sceneInfoDescriptorSet->GetDescriptorSet(),
				m_objectInfoDescriptorSet->GetDescriptorSet(),
				m_meshInfoDescriptorSet->GetDescriptorSet(),
				m_textureDescriptorSet->GetDescriptorSet()
			};
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0,
				static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, NULL);
			pushConstant.hiZSamplerID = m_hiZSamplerID;
			pushConstant.vbDispatchInfoCount = m_meshletSize;
			vkCmdPushConstants(commandBuffer, m_pipelineLayout,
				VkShaderStageFlagBits::VK_SHADER_STAGE_TASK_BIT_EXT,
				0, static_cast<uint32_t>(sizeof(pushConstant)), &pushConstant);
			uint32_t taskGroupSize = static_cast<uint32_t>(ceil(static_cast<float>(m_meshletSize) / 128.0f));
			m_vkCmdDrawMeshTasksEXT(commandBuffer, taskGroupSize, 1, 1);
		}
	}

	void TerrainPipeline::prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass)
	{
		// push constant
		std::vector<VkPushConstantRange> pushConstantRange = {
			Utility::Vulkan::CreateInfo::pushConstantRange(VkShaderStageFlagBits::VK_SHADER_STAGE_TASK_BIT_EXT,
				0,
				static_cast<uint32_t>(sizeof(pushConstant))),
		};

		// [pipeline]layout
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts = {
			m_sceneInfoDescriptorSet->GetDescriptorSetLayout(),
			m_objectInfoDescriptorSet->GetDescriptorSetLayout(),
			m_meshInfoDescriptorSet->GetDescriptorSetLayout(),
			m_textureDescriptorSet->GetDescriptorSetLayout()
		};
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = Utility::Vulkan::CreateInfo::pipelineLayoutCreateInfo(descriptorSetLayouts);
		pipelineLayoutCreateInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRange.size());
		pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRange.data();
		VK_VALIDATION(vkCreatePipelineLayout(*m_device, &pipelineLayoutCreateInfo, m_device->GetAllocationCallbacks(), &m_pipelineLayout));

		// pipeline
		std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo{
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_TASK_BIT_EXT, resources->GetShaderModules("TerrainVisibilityBuffer.task.spv")),
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT, resources->GetShaderModules("TerrainVisibilityBuffer.mesh.spv")),
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT, resources->GetShaderModules("TerrainVisibilityBuffer.frag.spv")),
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