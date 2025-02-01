// Copyright (c) 2025 kong9812
#include "StaticMesh.h"
#include <vector>

#include "ivma.h"
#include "AppInfo.h"
#include "VK_CreateInfo.h"
#include "VK_Validation.h"

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

		vmaDestroyBuffer(m_device->GetVmaAllocator(), m_staticMeshShaderObject.standardUBO.buffer.buffer, m_staticMeshShaderObject.standardUBO.buffer.allocation);

		vkDestroyDescriptorSetLayout(*m_device, m_staticMeshShaderObject.shaderBase.descriptorSetLayout, m_device->GetAllocationCallbacks());

		vkDestroyPipeline(*m_device, m_staticMeshShaderObject.shaderBase.pipeline, m_device->GetAllocationCallbacks());

		vkDestroyPipelineLayout(*m_device, m_staticMeshShaderObject.shaderBase.pipelineLayout, m_device->GetAllocationCallbacks());

		vkDestroyDescriptorPool(*m_device, m_descriptorPool, m_device->GetAllocationCallbacks());
	}

	void StaticMesh::PrepareForRender(RenderDevice_ptr device, RenderResources_ptr resources, VkRenderPass renderPass)
	{
		m_descriptorPool = VK_NULL_HANDLE;
		m_staticMeshShaderObject = {};

		m_device = device;
		m_resources = resources;
		m_renderPass = renderPass;
		m_currentLOD = LOD::Hide;
		m_lodDistances = { AppInfo::g_defaultLODVeryClose, AppInfo::g_defaultLODClose, AppInfo::g_defaultLODFar };
	}

	void StaticMesh::Update(const UpdateData& updateData, const Camera::CameraBase_ptr camera)
	{
		if (camera)
		{
			float distance = camera->GetDistance(m_transfrom.pos);
			if (distance <= m_lodDistances[LOD::LOD1])
			{
				m_currentLOD = LOD::LOD1;
			}
			else if (distance <= m_lodDistances[LOD::LOD2])
			{
				m_currentLOD = LOD::LOD2;
			}
			else if (distance <= m_lodDistances[LOD::LOD3])
			{
				m_currentLOD = LOD::LOD3;
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

		vkCmdBindDescriptorSets(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_staticMeshShaderObject.shaderBase.pipelineLayout, 0, 1, &m_staticMeshShaderObject.shaderBase.descriptorSet, 0, nullptr);
		vkCmdBindPipeline(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_staticMeshShaderObject.shaderBase.pipeline);

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
		{// standardUBO
			vmaTools::ShaderBufferAllocate(
				*m_device,
				m_device->GetVmaAllocator(),
				m_staticMeshShaderObject.standardUBO.data,
				VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				m_staticMeshShaderObject.standardUBO.buffer.buffer,
				m_staticMeshShaderObject.standardUBO.buffer.allocation,
				m_staticMeshShaderObject.standardUBO.buffer.allocationInfo,
				m_staticMeshShaderObject.standardUBO.buffer.descriptor);
		}
	}

	void StaticMesh::prepareDescriptors()
	{
		// pool(todo.今後はDescriptorPoolを一つにする予定)
		std::vector<VkDescriptorPoolSize> poolSize = {
			Utility::Vulkan::CreateInfo::descriptorPoolSize(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
		};
		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = Utility::Vulkan::CreateInfo::descriptorPoolCreateInfo(poolSize);
		VK_VALIDATION(vkCreateDescriptorPool(*m_device, &descriptorPoolCreateInfo, m_device->GetAllocationCallbacks(), &m_descriptorPool));

		{// m_staticMeshDescriptor
			// [descriptor]layout
			std::vector<VkDescriptorSetLayoutBinding> setLayoutBinding = {
				// binding: 0
				Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(0, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT),
			};
			VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = Utility::Vulkan::CreateInfo::descriptorSetLayoutCreateInfo(setLayoutBinding);
			VK_VALIDATION(vkCreateDescriptorSetLayout(*m_device, &descriptorSetLayoutCreateInfo, m_device->GetAllocationCallbacks(), &m_staticMeshShaderObject.shaderBase.descriptorSetLayout));
			// layout allocate
			VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = Utility::Vulkan::CreateInfo::descriptorSetAllocateInfo(m_descriptorPool, &m_staticMeshShaderObject.shaderBase.descriptorSetLayout);
			VK_VALIDATION(vkAllocateDescriptorSets(*m_device, &descriptorSetAllocateInfo, &m_staticMeshShaderObject.shaderBase.descriptorSet));
			// write descriptor set
			std::vector<VkWriteDescriptorSet> writeDescriptorSet = {
				Utility::Vulkan::CreateInfo::writeDescriptorSet(m_staticMeshShaderObject.shaderBase.descriptorSet, 0, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &m_staticMeshShaderObject.standardUBO.buffer.descriptor),
			};
			vkUpdateDescriptorSets(*m_device, static_cast<uint32_t>(writeDescriptorSet.size()), writeDescriptorSet.data(), 0, nullptr);
		}
	}

	void StaticMesh::prepareRenderPipeline()
	{
		// [pipeline]layout
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = Utility::Vulkan::CreateInfo::pipelineLayoutCreateInfo(&m_staticMeshShaderObject.shaderBase.descriptorSetLayout);
		VK_VALIDATION(vkCreatePipelineLayout(*m_device, &pipelineLayoutCreateInfo, m_device->GetAllocationCallbacks(), &m_staticMeshShaderObject.shaderBase.pipelineLayout));

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
		VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_TRUE, VkCompareOp::VK_COMPARE_OP_LESS_OR_EQUAL);
		VkPipelineColorBlendAttachmentState colorBlendAttachmentState = Utility::Vulkan::CreateInfo::pipelineColorBlendAttachmentState(VK_TRUE);
		VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineColorBlendStateCreateInfo(&colorBlendAttachmentState);
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
			m_staticMeshShaderObject.shaderBase.pipelineLayout,		// パイプラインレイアウト
			m_renderPass);											// レンダーパス
		VK_VALIDATION(vkCreateGraphicsPipelines(*m_device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, m_device->GetAllocationCallbacks(), &m_staticMeshShaderObject.shaderBase.pipeline));
	}
}
