// Copyright (c) 2025 kong9812
#include "RayTracingPipeline.h"
#include "VK_CreateInfo.h"
#include "AppInfo.h"
#include "MObject.h"
#include "TerrainIo.h"
#include "ImageIo.h"

#include "PrimitiveGeometryShape.h"

#include "CustomMesh.h"
#include "PrimitiveGeometry.h"
#include "Terrain.h"

namespace MyosotisFW::System::Render
{
	RayTracingPipeline::~RayTracingPipeline()
	{
		vkDestroyPipeline(*m_device, m_pipeline, m_device->GetAllocationCallbacks());
		vkDestroyPipelineLayout(*m_device, m_pipelineLayout, m_device->GetAllocationCallbacks());

		vmaDestroyBuffer(m_device->GetVmaAllocator(), m_vertexBuffer.buffer, m_vertexBuffer.allocation);
		vmaDestroyBuffer(m_device->GetVmaAllocator(), m_indexBuffer.buffer, m_indexBuffer.allocation);
	}

	void RayTracingPipeline::Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass)
	{
		prepareRenderPipeline(resources, renderPass);

		{// primitive geometry
			// vertex buffer
			Mesh mesh = Shape::createQuad();
			std::vector<uint32_t> index{};
			for (const Meshlet& meshlet : mesh.meshlet)
			{
				for (const uint32_t prim : meshlet.primitives)
				{
					index.push_back(meshlet.uniqueIndex[prim]);
				}
			}
			{// vertex
				VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(sizeof(VertexData) * mesh.vertex.size(), VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
				VmaAllocationCreateInfo allocationCreateInfo{};
				allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;	// CPUで更新可能
				VK_VALIDATION(vmaCreateBuffer(m_device->GetVmaAllocator(), &bufferCreateInfo, &allocationCreateInfo, &m_vertexBuffer.buffer, &m_vertexBuffer.allocation, &m_vertexBuffer.allocationInfo));
				m_vertexBuffer.descriptor = Utility::Vulkan::CreateInfo::descriptorBufferInfo(m_vertexBuffer.buffer);
				// mapping
				void* data{};
				VK_VALIDATION(vmaMapMemory(m_device->GetVmaAllocator(), m_vertexBuffer.allocation, &data));
				memcpy(data, mesh.vertex.data(), bufferCreateInfo.size);
				vmaUnmapMemory(m_device->GetVmaAllocator(), m_vertexBuffer.allocation);
			}
			{// index
				VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(sizeof(uint32_t) * index.size(), VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
				VmaAllocationCreateInfo allocationCreateInfo{};
				allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;	// CPUで更新可能
				VK_VALIDATION(vmaCreateBuffer(m_device->GetVmaAllocator(), &bufferCreateInfo, &allocationCreateInfo, &m_indexBuffer.buffer, &m_indexBuffer.allocation, &m_indexBuffer.allocationInfo));
				m_indexBuffer.descriptor = Utility::Vulkan::CreateInfo::descriptorBufferInfo(m_indexBuffer.buffer);

				// mapping
				void* data{};
				VK_VALIDATION(vmaMapMemory(m_device->GetVmaAllocator(), m_indexBuffer.allocation, &data));
				memcpy(data, index.data(), bufferCreateInfo.size);
				vmaUnmapMemory(m_device->GetVmaAllocator(), m_indexBuffer.allocation);
			}
		}

		// Buffer Device Address
		VkDeviceAddress vertexAddress = m_device->GetBufferDeviceAddress(m_vertexBuffer.buffer);
		VkDeviceAddress indexAddress = m_device->GetBufferDeviceAddress(m_indexBuffer.buffer);

		// AS Geometry Buffer Info (ASジオメトリ情報の設定)
		std::vector<VkAccelerationStructureGeometryKHR> accelerationStructureGeometryKHR{};
		VkAccelerationStructureGeometryKHR asGeometryKHR{};
		asGeometryKHR.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		asGeometryKHR.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		asGeometryKHR.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		asGeometryKHR.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
		asGeometryKHR.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		asGeometryKHR.geometry.triangles.vertexData.deviceAddress = vertexAddress;
		asGeometryKHR.geometry.triangles.vertexStride = sizeof(VertexData);
		asGeometryKHR.geometry.triangles.maxVertex = static_cast<uint32_t>(m_vertexBuffer.allocationInfo.size / sizeof(VertexData));
		asGeometryKHR.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
		asGeometryKHR.geometry.triangles.indexData.deviceAddress = indexAddress;
		accelerationStructureGeometryKHR.push_back(asGeometryKHR);

		// AS Build Geometry Info (TLAS構築情報の設定)
		VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfoKHR{};
		accelerationStructureBuildGeometryInfoKHR.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		accelerationStructureBuildGeometryInfoKHR.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		accelerationStructureBuildGeometryInfoKHR.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		accelerationStructureBuildGeometryInfoKHR.geometryCount = static_cast<uint32_t>(accelerationStructureGeometryKHR.size());
		accelerationStructureBuildGeometryInfoKHR.pGeometries = accelerationStructureGeometryKHR.data();

		// AS Instance Info (構築したBLASを参照するインスタンスバッファ)
		VkTransformMatrixKHR transformMatrix = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f };
		VkAccelerationStructureInstanceKHR accelerationStructureInstanceKHR{};
		accelerationStructureInstanceKHR.transform = transformMatrix;
		accelerationStructureInstanceKHR.instanceCustomIndex = 0;
		accelerationStructureInstanceKHR.mask = 0xFF;
		accelerationStructureInstanceKHR.instanceShaderBindingTableRecordOffset = 0;
		accelerationStructureInstanceKHR.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		accelerationStructureInstanceKHR.accelerationStructureReference = 0; // TODO: Bottom Level ASのデバイスアドレスを設定する
	}

	void RayTracingPipeline::BindCommandBuffer(const VkCommandBuffer& commandBuffer)
	{
		Buffer& currentVertexBuffer = m_vertexBuffer;
		Buffer& currentIndexBuffer = m_indexBuffer;

		vkCmdBindPipeline(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_pipeline);
		std::vector<VkDescriptorSet> descriptorSets = {
				m_sceneInfoDescriptorSet->GetDescriptorSet(),
				m_objectInfoDescriptorSet->GetDescriptorSet(),
				m_meshInfoDescriptorSet->GetDescriptorSet(),
				m_textureDescriptorSet->GetDescriptorSet()
		};
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0,
			static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, NULL);
		//vkCmdPushConstants(commandBuffer, m_pipelineLayout,
		//	VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT,
		//	0,
		//	static_cast<uint32_t>(sizeof(pushConstant)), &pushConstant);

		// todo. Ray Tracing commands
		//vkCmdTraceRaysKHR(commandBuffer);
	}

	void RayTracingPipeline::prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass)
	{
		//// push constant
		//std::vector<VkPushConstantRange> pushConstantRange = {
		//	// VS
		//	Utility::Vulkan::CreateInfo::pushConstantRange(VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT,
		//		0,
		//		static_cast<uint32_t>(sizeof(pushConstant))),
		//};

		// [pipeline]layout
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts = {
			m_sceneInfoDescriptorSet->GetDescriptorSetLayout(),
			m_objectInfoDescriptorSet->GetDescriptorSetLayout(),
			m_meshInfoDescriptorSet->GetDescriptorSetLayout(),
			m_textureDescriptorSet->GetDescriptorSetLayout()
		};
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = Utility::Vulkan::CreateInfo::pipelineLayoutCreateInfo(descriptorSetLayouts);
		//pipelineLayoutCreateInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRange.size());
		//pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRange.data();
		VK_VALIDATION(vkCreatePipelineLayout(*m_device, &pipelineLayoutCreateInfo, m_device->GetAllocationCallbacks(), &m_pipelineLayout));

		// pipeline
		std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo{
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT, resources->GetShaderModules("Lightmap.vert.spv")),
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT, resources->GetShaderModules("Lightmap.frag.spv")),
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
		VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = Utility::Vulkan::CreateInfo::pipelineRasterizationStateCreateInfo(VkPolygonMode::VK_POLYGON_MODE_FILL, VkCullModeFlagBits::VK_CULL_MODE_FRONT_BIT, VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
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