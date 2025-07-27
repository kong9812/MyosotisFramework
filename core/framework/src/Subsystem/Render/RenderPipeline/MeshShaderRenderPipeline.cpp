// Copyright (c) 2025 kong9812
#include "MeshShaderRenderPipeline.h"
#include "VK_CreateInfo.h"
#include "AppInfo.h"

#include "PrimitiveGeometryShape.h"

namespace MyosotisFW::System::Render
{
	MeshShaderRenderPipeline::~MeshShaderRenderPipeline()
	{
		vkFreeDescriptorSets(*m_device, m_descriptors->GetDescriptorPool(), 1, &m_descriptorSet);
		vkDestroyDescriptorSetLayout(*m_device, m_descriptorSetLayout, m_device->GetAllocationCallbacks());

		vmaDestroyBuffer(m_device->GetVmaAllocator(), m_vertexMetaDataBuffer.buffer, m_vertexMetaDataBuffer.allocation);
		vmaDestroyBuffer(m_device->GetVmaAllocator(), m_vertexDataBuffer.buffer, m_vertexDataBuffer.allocation);
		vmaDestroyBuffer(m_device->GetVmaAllocator(), m_indexMetaDataBuffer.buffer, m_indexMetaDataBuffer.allocation);
		vmaDestroyBuffer(m_device->GetVmaAllocator(), m_indexDataBuffer.buffer, m_indexDataBuffer.allocation);

		vkDestroyPipeline(*m_device, m_pipeline, m_device->GetAllocationCallbacks());
		vkDestroyPipelineLayout(*m_device, m_pipelineLayout, m_device->GetAllocationCallbacks());
	}

	void MeshShaderRenderPipeline::Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass)
	{
		m_vkCmdDrawMeshTasksEXT = (PFN_vkCmdDrawMeshTasksEXT)vkGetDeviceProcAddr(*m_device, "vkCmdDrawMeshTasksEXT");

		prepareDescriptorSet();
		prepareRenderPipeline(resources, renderPass);
	}

	void MeshShaderRenderPipeline::BindCommandBuffer(const VkCommandBuffer& commandBuffer)
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
		std::vector<VkDescriptorSet> descriptorSets = { m_descriptors->GetBindlessDescriptorSet(), m_descriptorSet };
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0,
			static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, NULL);
		uint32_t meshletGroupCount = 1;	// 仮 (これはMeshIDにするのもいいかも)
		m_vkCmdDrawMeshTasksEXT(commandBuffer, meshletGroupCount, 1, 1);
	}

	void MeshShaderRenderPipeline::prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass)
	{
		// [pipeline]layout
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts = { m_descriptors->GetBindlessDescriptorSetLayout(), m_descriptorSetLayout };
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = Utility::Vulkan::CreateInfo::pipelineLayoutCreateInfo(descriptorSetLayouts);
		VK_VALIDATION(vkCreatePipelineLayout(*m_device, &pipelineLayoutCreateInfo, m_device->GetAllocationCallbacks(), &m_pipelineLayout));

		// pipeline
		std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo{
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_TASK_BIT_EXT, resources->GetShaderModules("MeshShader.task.spv")),
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT, resources->GetShaderModules("MeshShader.mesh.spv")),
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT, resources->GetShaderModules("MeshShader.frag.spv")),
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

	void MeshShaderRenderPipeline::prepareDescriptorSet()
	{
		// [descriptor]layout
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBinding = {
			// binding: [SSBO] MetaData
			Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(0, VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT),
			// binding: [SSBO] RawData
			Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(1, VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT),
			// binding: [SSBO] MetaData
			Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(2, VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT),
			// binding: [SSBO] RawData
			Utility::Vulkan::CreateInfo::descriptorSetLayoutBinding(3, VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT),
		};

		// 未使用許可 & バインド後更新 を有効化
		std::vector<VkDescriptorBindingFlags> descriptorBindingFlags = {
			VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
			VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
			VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
			VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
		};
		VkDescriptorSetLayoutBindingFlagsCreateInfo descriptorSetLayoutBindingFlagsCreateInfo{};
		descriptorSetLayoutBindingFlagsCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		descriptorSetLayoutBindingFlagsCreateInfo.bindingCount = static_cast<uint32_t>(descriptorBindingFlags.size());
		descriptorSetLayoutBindingFlagsCreateInfo.pBindingFlags = descriptorBindingFlags.data();

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = Utility::Vulkan::CreateInfo::descriptorSetLayoutCreateInfo(setLayoutBinding);
		descriptorSetLayoutCreateInfo.flags = VkDescriptorSetLayoutCreateFlagBits::VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
		descriptorSetLayoutCreateInfo.pNext = &descriptorSetLayoutBindingFlagsCreateInfo;
		VK_VALIDATION(vkCreateDescriptorSetLayout(*m_device, &descriptorSetLayoutCreateInfo, m_device->GetAllocationCallbacks(), &m_descriptorSetLayout));
		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = Utility::Vulkan::CreateInfo::descriptorSetAllocateInfo(m_descriptors->GetDescriptorPool(), &m_descriptorSetLayout);
		VK_VALIDATION(vkAllocateDescriptorSets(*m_device, &descriptorSetAllocateInfo, &m_descriptorSet));

		// 仮MeshData (0番目のStaticMeshDataを再現する -- 現在の0番目はQuad)
		m_vertex = MyosotisFW::System::Render::Shape::createShape(Shape::PrimitiveGeometryShape::Quad,
			5.0f, glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));

		// SSBO作成 & writeDescriptorSet
		std::vector<VkWriteDescriptorSet> writeDescriptorSet{};

		//// vertexMetaData
		vmaTools::ShaderBufferObjectAllocate(
			*m_device,
			m_device->GetVmaAllocator(),
			static_cast<uint32_t>(sizeof(VertexDataMetaData)),	// 今は1つしかない　複数化対応が必要
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			m_vertexDataBuffer.buffer,
			m_vertexDataBuffer.allocation,
			m_vertexDataBuffer.allocationInfo,
			m_vertexDataBuffer.descriptor);
		VkDescriptorBufferInfo vertexMetaDataDescriptorBufferInfo = Utility::Vulkan::CreateInfo::descriptorBufferInfo(m_vertexDataBuffer.buffer, 0);
		writeDescriptorSet.push_back(Utility::Vulkan::CreateInfo::writeDescriptorSet(m_descriptorSet,
			0,	// 仮 set番号
			VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &vertexMetaDataDescriptorBufferInfo));
		// 0番目のMetaData
		VertexDataMetaData vertexMetaData{};
		vertexMetaData.vertexCount = m_vertex.vertex.size() / 13; // (x,y,z,w,uv1X....)
		vertexMetaData.primitiveCount = m_vertex.index.size() / 3; // 三角形
		vertexMetaData.vertexAttributeBit = Utility::Vulkan::CreateInfo::VertexAttributeBit::POSITION_VEC4 | Utility::Vulkan::CreateInfo::VertexAttributeBit::NORMAL | Utility::Vulkan::CreateInfo::VertexAttributeBit::UV | Utility::Vulkan::CreateInfo::VertexAttributeBit::COLOR_VEC4;
		vertexMetaData.unitSize = 13;
		vertexMetaData.offset = 0;	// 今後は計算しないといけない(追加されたときにindexを取るように)
		memcpy(m_vertexDataBuffer.allocationInfo.pMappedData, &vertexMetaData,
			static_cast<uint32_t>(sizeof(VertexDataMetaData)));	// 今は1つしかない　複数化対応が必要

		//// vertexData (全部の頂点データ)
		vmaTools::ShaderBufferObjectAllocate(
			*m_device,
			m_device->GetVmaAllocator(),
			static_cast<uint32_t>(sizeof(float)) * static_cast<uint32_t>(m_vertex.vertex.size()),	// quadの頂点総合サイズ
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			m_vertexMetaDataBuffer.buffer,
			m_vertexMetaDataBuffer.allocation,
			m_vertexMetaDataBuffer.allocationInfo,
			m_vertexMetaDataBuffer.descriptor);
		VkDescriptorBufferInfo vertexDataDescriptorBufferInfo = Utility::Vulkan::CreateInfo::descriptorBufferInfo(m_vertexMetaDataBuffer.buffer, 0);
		writeDescriptorSet.push_back(Utility::Vulkan::CreateInfo::writeDescriptorSet(m_descriptorSet,
			1,	// 仮 set番号
			VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &vertexDataDescriptorBufferInfo));
		memcpy(m_vertexMetaDataBuffer.allocationInfo.pMappedData, m_vertex.vertex.data(),
			static_cast<uint32_t>(sizeof(float)) * static_cast<uint32_t>(m_vertex.vertex.size()));	// quadの頂点総合サイズ

		//// indexMetaData
		vmaTools::ShaderBufferObjectAllocate(
			*m_device,
			m_device->GetVmaAllocator(),
			static_cast<uint32_t>(sizeof(IndexDataMetaData)),	// 今は1つしかない　複数化対応が必要
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			m_indexMetaDataBuffer.buffer,
			m_indexMetaDataBuffer.allocation,
			m_indexMetaDataBuffer.allocationInfo,
			m_indexMetaDataBuffer.descriptor);
		VkDescriptorBufferInfo indexMetaDataDescriptorBufferInfo = Utility::Vulkan::CreateInfo::descriptorBufferInfo(m_indexMetaDataBuffer.buffer, 0);
		writeDescriptorSet.push_back(Utility::Vulkan::CreateInfo::writeDescriptorSet(m_descriptorSet,
			2,	// 仮 set番号
			VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &indexMetaDataDescriptorBufferInfo));
		// 0番目のMetaData
		IndexDataMetaData indexMetaData{};
		indexMetaData.offset = 0;	// 今後は計算しないといけない(追加されたときにindexを取るように)
		memcpy(m_indexMetaDataBuffer.allocationInfo.pMappedData, &indexMetaData,
			static_cast<uint32_t>(sizeof(IndexDataMetaData)));	// 今は1つしかない　複数化対応が必要

		//// indexData (全部のindex)
		vmaTools::ShaderBufferObjectAllocate(
			*m_device,
			m_device->GetVmaAllocator(),
			static_cast<uint32_t>(sizeof(uint32_t)) * static_cast<uint32_t>(m_vertex.index.size()),	// quadのindex総合サイズ
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			m_indexDataBuffer.buffer,
			m_indexDataBuffer.allocation,
			m_indexDataBuffer.allocationInfo,
			m_indexDataBuffer.descriptor);
		VkDescriptorBufferInfo indexDataDescriptorBufferInfo = Utility::Vulkan::CreateInfo::descriptorBufferInfo(m_indexDataBuffer.buffer, 0);
		writeDescriptorSet.push_back(Utility::Vulkan::CreateInfo::writeDescriptorSet(m_descriptorSet,
			3,	// 仮 set番号
			VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &indexDataDescriptorBufferInfo));
		memcpy(m_indexDataBuffer.allocationInfo.pMappedData, m_vertex.index.data(),
			static_cast<uint32_t>(sizeof(uint32_t)) * static_cast<uint32_t>(m_vertex.index.size()));	// quadのindex総合サイズ

		// GPUへ!
		vkUpdateDescriptorSets(*m_device, static_cast<uint32_t>(writeDescriptorSet.size()), writeDescriptorSet.data(), 0, nullptr);
	}
}