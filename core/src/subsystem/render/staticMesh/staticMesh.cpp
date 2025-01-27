// Copyright (c) 2025 kong9812
#include "staticMesh.h"
#include <vector>

#include "ivma.h"
#include "appInfo.h"
#include "vkCreateInfo.h"
#include "vkValidation.h"

namespace MyosotisFW::System::Render
{
	StaticMesh::StaticMesh() : ObjectBase()
	{
		m_name = "スタティックメッシュ";
		m_transfrom.scale = glm::vec3(1.0f);
	}

	StaticMesh::~StaticMesh()
	{
		for (uint32_t logType = 0; logType < LOD::Max; logType++)
		{
			for (uint32_t meshIdx = 0; meshIdx < m_vertexBuffer[m_currentLOD].size(); meshIdx++)
			{
				vmaDestroyBuffer(m_device->GetVmaAllocator(), m_vertexBuffer[logType][meshIdx].buffer, m_vertexBuffer[logType][meshIdx].allocation);
				vmaDestroyBuffer(m_device->GetVmaAllocator(), m_indexBuffer[logType][meshIdx].buffer, m_indexBuffer[logType][meshIdx].allocation);
			}
		}

		vmaDestroyBuffer(m_device->GetVmaAllocator(), m_uboBuffer.buffer, m_uboBuffer.allocation);
		vkDestroyPipeline(*m_device, m_pipeline, m_device->GetAllocationCallbacks());
		vkDestroyPipelineLayout(*m_device, m_pipelineLayout, m_device->GetAllocationCallbacks());
		vkDestroyDescriptorSetLayout(*m_device, m_descriptorSetLayout, m_device->GetAllocationCallbacks());
		vkDestroyDescriptorPool(*m_device, m_descriptorPool, m_device->GetAllocationCallbacks());
	}

	void StaticMesh::PrepareForRender(RenderDevice_ptr device, RenderResources_ptr resources, VkRenderPass renderPass, VkPipelineCache pipelineCache)
	{
		m_descriptorPool = VK_NULL_HANDLE;
		m_descriptorSet = VK_NULL_HANDLE;
		m_descriptorSetLayout = VK_NULL_HANDLE;
		m_pipelineLayout = VK_NULL_HANDLE;
		m_pipeline = VK_NULL_HANDLE;
		m_ubo = {};
		m_uboBuffer = {};

		m_device = device;
		m_resources = resources;
		m_renderPass = renderPass;
		m_pipelineCache = pipelineCache;
		m_currentLOD = LOD::Hide;
		m_lodDistances = { AppInfo::g_defaultLODVeryClose, AppInfo::g_defaultLODClose, AppInfo::g_defaultLODFar };
	}

	void StaticMesh::Update(const Utility::Vulkan::Struct::UpdateData& updateData, const Camera::CameraBase_ptr camera)
	{
		if (camera)
		{
			float distance = camera->GetDistance(m_transfrom.pos);
			if (distance <= m_lodDistances[LOD::VeryClose])
			{
				m_currentLOD = LOD::VeryClose;
			}
			else if (distance <= m_lodDistances[LOD::Close])
			{
				m_currentLOD = LOD::Close;
			}
			else if (distance <= m_lodDistances[LOD::Far])
			{
				m_currentLOD = LOD::Far;
			}
			else
			{
				m_currentLOD = LOD::Hide;
			}
		}
	}

	void StaticMesh::BindCommandBuffer(VkCommandBuffer commandBuffer)
	{
		if ((m_currentLOD == LOD::Hide) || (!m_isReady)) return;

		vkCmdBindDescriptorSets(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);
		vkCmdBindPipeline(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

		const VkDeviceSize offsets[1] = { 0 };
		for (uint32_t meshIdx = 0; meshIdx < m_vertexBuffer[m_currentLOD].size(); meshIdx++)
		{
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuffer[m_currentLOD][meshIdx].buffer, offsets);
			vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer[m_currentLOD][meshIdx].buffer, 0, VkIndexType::VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer, m_indexBuffer[m_currentLOD][meshIdx].allocationInfo.size / sizeof(uint32_t), 1, 0, 0, 0);
		}
	}

	rapidjson::Value StaticMesh::Serialize(rapidjson::Document::AllocatorType& allocator) const
	{
		rapidjson::Value obj = __super::Serialize(allocator);
		return obj;
	}

	void StaticMesh::prepareUniformBuffers()
	{
		VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(sizeof(m_ubo), VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		VmaAllocationCreateInfo allocationCreateInfo{};
		allocationCreateInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU;					// CPUで更新可能
		allocationCreateInfo.flags = VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT;	// 永続マッピング
		VK_VALIDATION(vmaCreateBuffer(m_device->GetVmaAllocator(), &bufferCreateInfo, &allocationCreateInfo, &m_uboBuffer.buffer, &m_uboBuffer.allocation, &m_uboBuffer.allocationInfo));
		m_uboBuffer.descriptor = Utility::Vulkan::CreateInfo::descriptorBufferInfo(m_uboBuffer.buffer);
	}

	void StaticMesh::prepareDescriptors()
	{
		// pool(todo.今後はDescriptorPoolを一つにする予定)
		std::vector<VkDescriptorPoolSize> poolSize = {
			Utility::Vulkan::CreateInfo::descriptorPoolSize(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
		};
		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = Utility::Vulkan::CreateInfo::descriptorPoolCreateInfo(poolSize);
		VK_VALIDATION(vkCreateDescriptorPool(*m_device, &descriptorPoolCreateInfo, m_device->GetAllocationCallbacks(), &m_descriptorPool));

		// [descriptor]layout
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBinding = {
			// binding: 0
			Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(0, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT),
		};
		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = Utility::Vulkan::CreateInfo::descriptorSetLayoutCreateInfo(setLayoutBinding);
		VK_VALIDATION(vkCreateDescriptorSetLayout(*m_device, &descriptorSetLayoutCreateInfo, m_device->GetAllocationCallbacks(), &m_descriptorSetLayout));
		// layout allocate
		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = Utility::Vulkan::CreateInfo::descriptorSetAllocateInfo(m_descriptorPool, &m_descriptorSetLayout);
		VK_VALIDATION(vkAllocateDescriptorSets(*m_device, &descriptorSetAllocateInfo, &m_descriptorSet));

		// write descriptor set
		std::vector<VkWriteDescriptorSet> writeDescriptorSet = {
			Utility::Vulkan::CreateInfo::writeDescriptorSet(m_descriptorSet, 0, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &m_uboBuffer.descriptor),
		};
		vkUpdateDescriptorSets(*m_device, static_cast<uint32_t>(writeDescriptorSet.size()), writeDescriptorSet.data(), 0, nullptr);
	}

	void StaticMesh::prepareRenderPipeline()
	{
		// [pipeline]layout
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = Utility::Vulkan::CreateInfo::pipelineLayoutCreateInfo(&m_descriptorSetLayout);
		VK_VALIDATION(vkCreatePipelineLayout(*m_device, &pipelineLayoutCreateInfo, m_device->GetAllocationCallbacks(), &m_pipelineLayout));

		// pipeline
		std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo{
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT, m_resources->GetShaderModules("StaticMesh_DepthFade.vert.spv")),
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT, m_resources->GetShaderModules("StaticMesh_DepthFade.frag.spv")),
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
		VkPipelineColorBlendAttachmentState colorBlendAttachmentState = Utility::Vulkan::CreateInfo::pipelineColorBlendAttachmentState(VK_TRUE);
		VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineColorBlendStateCreateInfo(&colorBlendAttachmentState);
		std::vector<VkDynamicState> dynamicStates = { VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT, VkDynamicState::VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineDynamicStateCreateInfo(dynamicStates);

		VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = Utility::Vulkan::CreateInfo::graphicsPipelineCreateInfo(
			shaderStageCreateInfo,						// シェーダーステージ
			&pipelineVertexInputStateCreateInfo,		// 頂点入力
			&inputAssemblyStateCreateInfo,				// 入力アセンブリ
			&viewportStateCreateInfo,					// ビューポートステート
			&rasterizationStateCreateInfo,				// ラスタライゼーション
			&multisampleStateCreateInfo,				// マルチサンプリング
			&depthStencilStateCreateInfo,				// 深度/ステンシル
			&colorBlendStateCreateInfo,					// カラーブレンディング
			&dynamicStateCreateInfo,					// 動的状態
			m_pipelineLayout,							// パイプラインレイアウト
			m_renderPass);								// レンダーパス
		VK_VALIDATION(vkCreateGraphicsPipelines(*m_device, m_pipelineCache, 1, &graphicsPipelineCreateInfo, m_device->GetAllocationCallbacks(), &m_pipeline));
	}
}
