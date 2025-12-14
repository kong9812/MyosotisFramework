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

		// SBT構築
		VkStridedDeviceAddressRegionKHR raygenSBT{};
		raygenSBT.deviceAddress = 0; // TODO: Raygen SBTのデバイスアドレスを設定する
		raygenSBT.stride = 0; // TODO: Raygen SBTのエントリサイズを設定する
		raygenSBT.size = 0; // TODO: Raygen SBTの全体サイズを設定する
		VkStridedDeviceAddressRegionKHR missSBT{};
		VkStridedDeviceAddressRegionKHR hitSBT{};

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

		vkCmdTraceRaysKHR(commandBuffer,
			&m_raygenSBTBuffer.region,
			&m_missSBTBuffer.region,
			&m_hitSBTBuffer.region,
			nullptr,
			AppInfo::g_windowWidth,
			AppInfo::g_windowHeight,
			1);
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
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_KHR, resources->GetShaderModules("RayTracing.rgen.spv")),
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, resources->GetShaderModules("RayTracing.rchit.spv")),
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_MISS_BIT_KHR, resources->GetShaderModules("RayTracing.rmiss.spv")),
		};

		// rayTracingShaderGroupCreateInfoKHR
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> rayTracingShaderGroupCreateInfoKHR{};
		VkRayTracingShaderGroupCreateInfoKHR raygenShaderGroup{};
		raygenShaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		raygenShaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		raygenShaderGroup.generalShader = 0;
		VkRayTracingShaderGroupCreateInfoKHR rchitShaderGroup{};
		raygenShaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		raygenShaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		raygenShaderGroup.generalShader = 1;
		VkRayTracingShaderGroupCreateInfoKHR rmissShaderGroup{};
		rmissShaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		rmissShaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		rmissShaderGroup.generalShader = 2;
		rayTracingShaderGroupCreateInfoKHR.push_back(raygenShaderGroup);
		rayTracingShaderGroupCreateInfoKHR.push_back(rchitShaderGroup);
		rayTracingShaderGroupCreateInfoKHR.push_back(rmissShaderGroup);

		VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCreateInfoKHR{};
		rayTracingPipelineCreateInfoKHR.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
		rayTracingPipelineCreateInfoKHR.stageCount = static_cast<uint32_t>(shaderStageCreateInfo.size());
		rayTracingPipelineCreateInfoKHR.pStages = shaderStageCreateInfo.data();
		rayTracingPipelineCreateInfoKHR.groupCount = static_cast<uint32_t>(rayTracingShaderGroupCreateInfoKHR.size());
		rayTracingPipelineCreateInfoKHR.pGroups = rayTracingShaderGroupCreateInfoKHR.data();
		rayTracingPipelineCreateInfoKHR.maxPipelineRayRecursionDepth = 1;
		rayTracingPipelineCreateInfoKHR.layout = m_pipelineLayout;
		VK_VALIDATION(vkCreateRayTracingPipelinesKHR(*m_device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineCreateInfoKHR, m_device->GetAllocationCallbacks(), &m_pipeline));

		// SBTの作成
		createShaderBindingTable();
	}

	void RayTracingPipeline::createShaderBindingTable()
	{
		// Ray Tracing Pipeline Properties
		const VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingProperties = m_device->GetPhysicalDeviceRayTracingPipelinePropertiesKHR();

		// Shader handle size
		const uint32_t handleSize = rayTracingProperties.shaderGroupHandleSize;
		const uint32_t handleAlignment = rayTracingProperties.shaderGroupHandleAlignment;
		const uint32_t handleSizeAligned = (handleSize + handleAlignment - 1) & ~(handleAlignment - 1);

		const uint32_t groupCount = 3; // raygen, miss, chit
		const uint32_t sbtSize = groupCount * handleSizeAligned;

		// 全シェーダーグループハンドルの取得
		std::vector<uint8_t> shaderHandleStorage(sbtSize);
		VK_VALIDATION(vkGetRayTracingShaderGroupHandlesKHR(
			*m_device,
			m_pipeline,
			0,				// 最初のシェーダーグループインデックス
			groupCount,		// 取得するシェーダーグループ数
			sbtSize,		// バッファサイズ
			shaderHandleStorage.data()));

		// raygen SBTの作成
		const size_t raygenOffset = handleSizeAligned;
		createSBTBuffer(handleSizeAligned, &m_raygenSBTBuffer.sbtBuffer, shaderHandleStorage.data() + 0 * handleSize);
		// Regionの設定
		m_raygenSBTBuffer.region.deviceAddress = m_device->GetBufferDeviceAddress(m_raygenSBTBuffer.sbtBuffer.buffer);
		m_raygenSBTBuffer.region.stride = handleSizeAligned;
		m_raygenSBTBuffer.region.size = raygenOffset;

		// miss SBTの作成
		const size_t missOffset = handleSizeAligned;
		createSBTBuffer(handleSizeAligned, &m_missSBTBuffer.sbtBuffer, shaderHandleStorage.data() + 1 * handleSize);
		// Regionの設定
		m_missSBTBuffer.region.deviceAddress = m_device->GetBufferDeviceAddress(m_missSBTBuffer.sbtBuffer.buffer);
		m_missSBTBuffer.region.stride = handleSizeAligned;
		m_missSBTBuffer.region.size = missOffset;

		// chit SBTの作成
		const size_t chitOffset = handleSizeAligned;
		createSBTBuffer(handleSizeAligned, &m_hitSBTBuffer.sbtBuffer, shaderHandleStorage.data() + 2 * handleSize);
		// Regionの設定
		m_hitSBTBuffer.region.deviceAddress = m_device->GetBufferDeviceAddress(m_hitSBTBuffer.sbtBuffer.buffer);
		m_hitSBTBuffer.region.stride = handleSizeAligned;
		m_hitSBTBuffer.region.size = chitOffset;
	}

	void RayTracingPipeline::createSBTBuffer(VkDeviceSize size, Buffer* sbtBuffer, const void* data)
	{
		VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(
			size,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		VmaAllocationCreateInfo allocationCreateInfo{};
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;	// CPUで更新可能
		VK_VALIDATION(vmaCreateBuffer(m_device->GetVmaAllocator(), &bufferCreateInfo, &allocationCreateInfo, &sbtBuffer->buffer, &sbtBuffer->allocation, &sbtBuffer->allocationInfo));
		sbtBuffer->descriptor = Utility::Vulkan::CreateInfo::descriptorBufferInfo(sbtBuffer->buffer);
		// mapping
		void* mappedData{};
		VK_VALIDATION(vmaMapMemory(m_device->GetVmaAllocator(), sbtBuffer->allocation, &mappedData));
		memcpy(mappedData, data, size);
		vmaUnmapMemory(m_device->GetVmaAllocator(), sbtBuffer->allocation);
	}
}