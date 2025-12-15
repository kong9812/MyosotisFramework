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

		// BLAS構築
		createBLAS();

		// TLAS構築
		createTLAS();

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
		// [pipeline]layout
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts = {
			m_sceneInfoDescriptorSet->GetDescriptorSetLayout(),
			m_objectInfoDescriptorSet->GetDescriptorSetLayout(),
			m_meshInfoDescriptorSet->GetDescriptorSetLayout(),
			m_textureDescriptorSet->GetDescriptorSetLayout()
		};
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = Utility::Vulkan::CreateInfo::pipelineLayoutCreateInfo(descriptorSetLayouts);
		VK_VALIDATION(vkCreatePipelineLayout(*m_device, &pipelineLayoutCreateInfo, m_device->GetAllocationCallbacks(), &m_pipelineLayout));

		// shader stages
		std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo{
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_KHR, resources->GetShaderModules("RayTracing.rgen.spv")),
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, resources->GetShaderModules("RayTracing.rchit.spv")),
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_MISS_BIT_KHR, resources->GetShaderModules("RayTracing.rmiss.spv")),
		};
		// shader groups
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> rayTracingShaderGroupCreateInfoKHR{
			Utility::Vulkan::CreateInfo::rayTracingShaderGroupCreateInfo(VkRayTracingShaderGroupTypeKHR::VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR, 0),
			Utility::Vulkan::CreateInfo::rayTracingShaderGroupCreateInfo(VkRayTracingShaderGroupTypeKHR::VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR, 1),
			Utility::Vulkan::CreateInfo::rayTracingShaderGroupCreateInfo(VkRayTracingShaderGroupTypeKHR::VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR, VK_SHADER_UNUSED_KHR, 2),
		};

		// Pipeline CI
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
		uint32_t raygenHandleCount = 1;
		createSBTBuffer(m_raygenSBTBuffer.sbtBuffer, handleSize, raygenHandleCount);
		m_raygenSBTBuffer.region.deviceAddress = m_device->GetBufferDeviceAddress(m_raygenSBTBuffer.sbtBuffer.buffer);
		m_raygenSBTBuffer.region.stride = handleSizeAligned;
		m_raygenSBTBuffer.region.size = handleSizeAligned * raygenHandleCount;
		// miss SBTの作成
		uint32_t missHandleCount = 1;
		createSBTBuffer(m_missSBTBuffer.sbtBuffer, handleSize, missHandleCount);
		m_missSBTBuffer.region.deviceAddress = m_device->GetBufferDeviceAddress(m_missSBTBuffer.sbtBuffer.buffer);
		m_missSBTBuffer.region.stride = handleSizeAligned;
		m_missSBTBuffer.region.size = handleSizeAligned * missHandleCount;
		// chit SBTの作成
		uint32_t chitHandleCount = 1;
		createSBTBuffer(m_hitSBTBuffer.sbtBuffer, handleSize, chitHandleCount);
		m_hitSBTBuffer.region.deviceAddress = m_device->GetBufferDeviceAddress(m_hitSBTBuffer.sbtBuffer.buffer);
		m_hitSBTBuffer.region.stride = handleSizeAligned;
		m_hitSBTBuffer.region.size = handleSizeAligned * chitHandleCount;

		// copy handle
		memcpy(m_raygenSBTBuffer.sbtBuffer.allocationInfo.pMappedData, shaderHandleStorage.data() + 0, handleSize);
		memcpy(m_missSBTBuffer.sbtBuffer.allocationInfo.pMappedData, shaderHandleStorage.data() + (raygenHandleCount * handleSizeAligned), handleSize);
		memcpy(m_hitSBTBuffer.sbtBuffer.allocationInfo.pMappedData, shaderHandleStorage.data() + ((raygenHandleCount + missHandleCount) * handleSizeAligned), handleSize);
	}

	void RayTracingPipeline::createSBTBuffer(Buffer& buffer, const uint32_t handleSize, const uint32_t handleCount)
	{
		const VkDeviceSize sbtSize = handleSize * handleCount;
		// SBTバッファの作成
		vmaTools::CreateBuffer(
			*m_device,
			m_device->GetVmaAllocator(),
			sbtSize,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR |
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			buffer.buffer,
			buffer.allocation,
			buffer.allocationInfo,
			buffer.descriptor);
	}

	void RayTracingPipeline::createBLAS()
	{
		// primitive geometry
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

		// [仮] ObjectTransform
		VkTransformMatrixKHR transformMatrix = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f };

		// TransformBuffer作成
		Buffer transformBuffer{}; // tmp
		vmaTools::CreateBuffer(
			*m_device,
			m_device->GetVmaAllocator(),
			sizeof(VkTransformMatrixKHR),
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			transformBuffer.buffer,
			transformBuffer.allocation,
			transformBuffer.allocationInfo,
			transformBuffer.descriptor);
		// DeviceAddress (vertex,index,transform)
		VkDeviceOrHostAddressConstKHR vertexAddress{};
		vertexAddress.deviceAddress = m_device->GetBufferDeviceAddress(m_vertexBuffer.buffer);
		VkDeviceOrHostAddressConstKHR indexAddress{};
		indexAddress.deviceAddress = m_device->GetBufferDeviceAddress(m_indexBuffer.buffer);
		VkDeviceOrHostAddressConstKHR transformAddress{};
		transformAddress.deviceAddress = m_device->GetBufferDeviceAddress(transformBuffer.buffer);

		// AS Geometry
		VkAccelerationStructureGeometryKHR asGeometryKHR{};
		asGeometryKHR.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		asGeometryKHR.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		asGeometryKHR.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
		asGeometryKHR.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		asGeometryKHR.geometry.triangles.vertexData = vertexAddress;
		asGeometryKHR.geometry.triangles.maxVertex = static_cast<uint32_t>(mesh.vertex.size());
		asGeometryKHR.geometry.triangles.vertexStride = sizeof(VertexData);
		asGeometryKHR.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
		asGeometryKHR.geometry.triangles.indexData = indexAddress;
		asGeometryKHR.geometry.triangles.transformData = transformAddress;

		// プリミティブ数
		const uint32_t primitiveCount = static_cast<uint32_t>(index.size() / 3);

		// ASのビルド範囲
		VkAccelerationStructureBuildRangeInfoKHR asBuildRangeInfoKHR{};
		asBuildRangeInfoKHR.firstVertex = 0;
		asBuildRangeInfoKHR.primitiveOffset = 0;
		asBuildRangeInfoKHR.primitiveCount = primitiveCount;
		asBuildRangeInfoKHR.transformOffset = 0;

		// ASのビルドジオメトリ情報
		VkAccelerationStructureBuildGeometryInfoKHR asBuildGeometryInfoKHR{};
		asBuildGeometryInfoKHR.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		asBuildGeometryInfoKHR.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		asBuildGeometryInfoKHR.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		asBuildGeometryInfoKHR.geometryCount = 1;	// ジオメトリ数(今は1つだけ)
		asBuildGeometryInfoKHR.pGeometries = &asGeometryKHR;

		// ASのビルドサイズ
		VkAccelerationStructureBuildSizesInfoKHR asBuildSizesInfoKHR{};
		asBuildSizesInfoKHR.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
		vkGetAccelerationStructureBuildSizesKHR(
			*m_device,
			VkAccelerationStructureBuildTypeKHR::VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
			&asBuildGeometryInfoKHR,
			&primitiveCount,
			&asBuildSizesInfoKHR);

		// BLAS Bufferの作成
		Buffer blasBuffer{};						// tmp
		VkAccelerationStructureKHR blasHandle{};	// tmp
		VkDeviceAddress blasDeviceAddress{};		// tmp
		vmaTools::CreateASBuffer(
			*m_device,
			m_device->GetVmaAllocator(),
			asBuildSizesInfoKHR.accelerationStructureSize,
			blasBuffer.buffer,
			blasBuffer.allocation,
			blasBuffer.allocationInfo,
			blasBuffer.descriptor);

		// ASの作成
		VkAccelerationStructureCreateInfoKHR asCI{};
		asCI.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		asCI.buffer = blasBuffer.buffer;
		asCI.size = asBuildSizesInfoKHR.accelerationStructureSize;
		asCI.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		VK_VALIDATION(vkCreateAccelerationStructureKHR(*m_device, &asCI, m_device->GetAllocationCallbacks(), &blasHandle));

		// Scratch Bufferの作成
		Buffer scratchBuffer{}; // tmp
		vmaTools::CreateScratchBuffer(
			*m_device,
			m_device->GetVmaAllocator(),
			asBuildSizesInfoKHR.buildScratchSize,
			scratchBuffer.buffer,
			scratchBuffer.allocation,
			scratchBuffer.allocationInfo,
			scratchBuffer.descriptor);
		VkDeviceAddress scratchAddress = m_device->GetBufferDeviceAddress(scratchBuffer.buffer);

		// asBuildGeometryInfoKHRの情報追加
		asBuildGeometryInfoKHR.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		asBuildGeometryInfoKHR.dstAccelerationStructure = blasHandle;
		asBuildGeometryInfoKHR.scratchData.deviceAddress = scratchAddress;

		// CommandBufferの準備
		const VkAccelerationStructureBuildRangeInfoKHR* pAsBuildRangeInfoKHR = &asBuildRangeInfoKHR;
		VkCommandBuffer commandBuffer = m_device->GetGraphicsQueue()->AllocateSingleUseCommandBuffer(*m_device);
		vkCmdBuildAccelerationStructuresKHR(
			commandBuffer,
			1,
			&asBuildGeometryInfoKHR,
			&pAsBuildRangeInfoKHR);
		VkSubmitInfo submitInfo = Utility::Vulkan::CreateInfo::submitInfo();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		// fence
		VkFence fence{};
		VkFenceCreateInfo fenceCreateInfo = Utility::Vulkan::CreateInfo::fenceCreateInfo();
		VK_VALIDATION(vkCreateFence(*m_device, &fenceCreateInfo, m_device->GetAllocationCallbacks(), &fence));
		// submit
		m_device->GetGraphicsQueue()->Submit(submitInfo, fence);
		// wait
		VK_VALIDATION(vkWaitForFences(*m_device, 1, &fence, VK_TRUE, UINT64_MAX));
		// 後片付け
		vkDestroyFence(*m_device, fence, m_device->GetAllocationCallbacks());
		m_device->GetGraphicsQueue()->FreeSingleUseCommandBuffer(*m_device, commandBuffer);

		// ASのデバイスアドレス取得
		VkAccelerationStructureDeviceAddressInfoKHR asDeviceAddressInfo{};
		asDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		asDeviceAddressInfo.accelerationStructure = blasHandle;
		blasDeviceAddress = vkGetAccelerationStructureDeviceAddressKHR(*m_device, &asDeviceAddressInfo);

		// ScratchBufferの破棄
		vmaDestroyBuffer(m_device->GetVmaAllocator(), scratchBuffer.buffer, scratchBuffer.allocation);
	}

	void RayTracingPipeline::createTLAS()
	{
		// 仮
		VkDeviceAddress blasDeviceAddress{}; // tmp

		// [仮] SceneTransform
		VkTransformMatrixKHR transformMatrix = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f };

		// AS Instance Info (構築したBLASを参照するインスタンスバッファ)
		VkAccelerationStructureInstanceKHR asInstanceKHR{};
		asInstanceKHR.transform = transformMatrix;
		asInstanceKHR.instanceCustomIndex = 0;
		asInstanceKHR.mask = 0xFF;
		asInstanceKHR.instanceShaderBindingTableRecordOffset = 0;
		asInstanceKHR.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		asInstanceKHR.accelerationStructureReference = blasDeviceAddress;	// todo. 正しい物をセットするように！

		// AS Instance Bufferの作成
		Buffer instanceBuffer{}; // tmp
		vmaTools::CreateASInstanceBuffer(
			*m_device,
			m_device->GetVmaAllocator(),
			sizeof(VkAccelerationStructureInstanceKHR),
			instanceBuffer.buffer,
			instanceBuffer.allocation,
			instanceBuffer.allocationInfo,
			instanceBuffer.descriptor);
		VkDeviceOrHostAddressConstKHR instanceDataAddress{};
		instanceDataAddress.deviceAddress = m_device->GetBufferDeviceAddress(instanceBuffer.buffer);

		// AS Geometry
		VkAccelerationStructureGeometryKHR asGeometryKHR{};
		asGeometryKHR.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		asGeometryKHR.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		asGeometryKHR.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		asGeometryKHR.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		asGeometryKHR.geometry.instances.arrayOfPointers = VK_FALSE;
		asGeometryKHR.geometry.instances.data = instanceDataAddress;

		// ASのビルドジオメトリ情報
		VkAccelerationStructureBuildGeometryInfoKHR asBuildGeometryInfoKHR{};
		asBuildGeometryInfoKHR.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		asBuildGeometryInfoKHR.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		asBuildGeometryInfoKHR.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		asBuildGeometryInfoKHR.geometryCount = 1;
		asBuildGeometryInfoKHR.pGeometries = &asGeometryKHR;

		// プリミティブ数
		const uint32_t primitiveCount = 1;

		// ASのビルドサイズ情報
		VkAccelerationStructureBuildSizesInfoKHR asBuildSizesInfoKHR{};
		asBuildSizesInfoKHR.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
		vkGetAccelerationStructureBuildSizesKHR(
			*m_device,
			VkAccelerationStructureBuildTypeKHR::VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
			&asBuildGeometryInfoKHR,
			&primitiveCount,
			&asBuildSizesInfoKHR);

		// TLAS Bufferの作成
		Buffer tlasBuffer{};						// tmp
		VkAccelerationStructureKHR tlasHandle{};	// tmp
		VkDeviceAddress tlasDeviceAddress{};		// tmp
		vmaTools::CreateASBuffer(
			*m_device,
			m_device->GetVmaAllocator(),
			asBuildSizesInfoKHR.accelerationStructureSize,
			tlasBuffer.buffer,
			tlasBuffer.allocation,
			tlasBuffer.allocationInfo,
			tlasBuffer.descriptor);

		// ASの作成
		VkAccelerationStructureCreateInfoKHR asCI{};
		asCI.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		asCI.buffer = tlasBuffer.buffer;
		asCI.size = asBuildSizesInfoKHR.accelerationStructureSize;
		asCI.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		VK_VALIDATION(vkCreateAccelerationStructureKHR(*m_device, &asCI, m_device->GetAllocationCallbacks(), &tlasHandle));

		// Scratch Bufferの作成
		Buffer scratchBuffer{}; // tmp
		vmaTools::CreateScratchBuffer(
			*m_device,
			m_device->GetVmaAllocator(),
			asBuildSizesInfoKHR.buildScratchSize,
			scratchBuffer.buffer,
			scratchBuffer.allocation,
			scratchBuffer.allocationInfo,
			scratchBuffer.descriptor);
		VkDeviceAddress scratchAddress = m_device->GetBufferDeviceAddress(scratchBuffer.buffer);

		// asBuildGeometryInfoKHRの情報追加
		asBuildGeometryInfoKHR.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		asBuildGeometryInfoKHR.dstAccelerationStructure = tlasHandle;
		asBuildGeometryInfoKHR.scratchData.deviceAddress = scratchAddress;
		asBuildGeometryInfoKHR.geometryCount = 1;
		asBuildGeometryInfoKHR.pGeometries = &asGeometryKHR;

		// ASのビルド範囲
		VkAccelerationStructureBuildRangeInfoKHR asBuildRangeInfoKHR{};
		asBuildRangeInfoKHR.firstVertex = 0;
		asBuildRangeInfoKHR.primitiveOffset = 0;
		asBuildRangeInfoKHR.primitiveCount = primitiveCount;
		asBuildRangeInfoKHR.transformOffset = 0;
		std::vector<VkAccelerationStructureBuildRangeInfoKHR*> pAsBuildRangeInfoKHRs = { &asBuildRangeInfoKHR };

		// CommandBufferの準備
		VkCommandBuffer commandBuffer = m_device->GetGraphicsQueue()->AllocateSingleUseCommandBuffer(*m_device);
		vkCmdBuildAccelerationStructuresKHR(
			commandBuffer,
			1,
			&asBuildGeometryInfoKHR,
			pAsBuildRangeInfoKHRs.data());
		VkSubmitInfo submitInfo = Utility::Vulkan::CreateInfo::submitInfo();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		// fence
		VkFence fence{};
		VkFenceCreateInfo fenceCreateInfo = Utility::Vulkan::CreateInfo::fenceCreateInfo();
		VK_VALIDATION(vkCreateFence(*m_device, &fenceCreateInfo, m_device->GetAllocationCallbacks(), &fence));
		// submit
		m_device->GetGraphicsQueue()->Submit(submitInfo, fence);
		// wait
		VK_VALIDATION(vkWaitForFences(*m_device, 1, &fence, VK_TRUE, UINT64_MAX));

		// 後片付け
		vkDestroyFence(*m_device, fence, m_device->GetAllocationCallbacks());
		m_device->GetGraphicsQueue()->FreeSingleUseCommandBuffer(*m_device, commandBuffer);

		// ASのデバイスアドレス取得
		VkAccelerationStructureDeviceAddressInfoKHR asDeviceAddressInfo{};
		asDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		asDeviceAddressInfo.accelerationStructure = tlasHandle;
		tlasDeviceAddress = vkGetAccelerationStructureDeviceAddressKHR(*m_device, &asDeviceAddressInfo);

		// ScratchBufferの破棄
		vmaDestroyBuffer(m_device->GetVmaAllocator(), scratchBuffer.buffer, scratchBuffer.allocation);
		// InstanceBufferの破棄
		vmaDestroyBuffer(m_device->GetVmaAllocator(), instanceBuffer.buffer, instanceBuffer.allocation);
	}
}