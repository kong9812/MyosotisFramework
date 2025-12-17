// Copyright (c) 2025 kong9812
#include "AccelerationStructureManager.h"
#include <algorithm>
#include "RenderDevice.h"
#include "RenderQueue.h"
#include "MObject.h"

namespace MyosotisFW::System::Render
{
	AccelerationStructureManager::AccelerationStructureManager(const RenderDevice_ptr& device) :
		m_device(device),
		m_blasDirty(false),
		m_tlasDirty(false),
		m_blas({}),
		m_tlas({}),
		m_pendingBLASBuild({}),
		m_vkGetRayTracingShaderGroupHandlesKHR(VK_NULL_HANDLE),
		m_vkCreateAccelerationStructureKHR(VK_NULL_HANDLE),
		m_vkCmdBuildAccelerationStructuresKHR(VK_NULL_HANDLE),
		m_vkGetAccelerationStructureDeviceAddressKHR(VK_NULL_HANDLE),
		m_vkGetAccelerationStructureBuildSizesKHR(VK_NULL_HANDLE),
		m_vkCmdTraceRaysKHR(VK_NULL_HANDLE),
		m_vkCreateRayTracingPipelinesKHR(VK_NULL_HANDLE) {
		// 関数ポインタの取得
		m_vkGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(*m_device, "vkGetRayTracingShaderGroupHandlesKHR"));
		m_vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(*m_device, "vkCreateAccelerationStructureKHR"));
		m_vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(*m_device, "vkCmdBuildAccelerationStructuresKHR"));
		m_vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(*m_device, "vkGetAccelerationStructureDeviceAddressKHR"));
		m_vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(*m_device, "vkGetAccelerationStructureBuildSizesKHR"));
		m_vkCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(*m_device, "vkCmdTraceRaysKHR"));
		m_vkCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(*m_device, "vkCreateRayTracingPipelinesKHR"));
		m_vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(*m_device, "vkDestroyAccelerationStructureKHR"));
	}

	AccelerationStructureManager::~AccelerationStructureManager()
	{
		for (const BLASInfo& blasInfo : m_pendingBLASBuild)
		{
			vmaDestroyBuffer(m_device->GetVmaAllocator(), blasInfo.vertexBuffer.buffer, blasInfo.vertexBuffer.allocation);
			vmaDestroyBuffer(m_device->GetVmaAllocator(), blasInfo.indexBuffer.buffer, blasInfo.indexBuffer.allocation);
			vmaDestroyBuffer(m_device->GetVmaAllocator(), blasInfo.transformBuffer.buffer, blasInfo.transformBuffer.allocation);

			vmaDestroyBuffer(m_device->GetVmaAllocator(), blasInfo.blas.buffer.buffer, blasInfo.blas.buffer.allocation);

			m_vkDestroyAccelerationStructureKHR(*m_device, blasInfo.blas.handle, m_device->GetAllocationCallbacks());
		}
	}

	void AccelerationStructureManager::OnLoadedMesh(const std::vector<Mesh>& meshes)
	{
		for (const Mesh& mesh : meshes)
		{
			// BLAS Info
			BLASInfo blasInfo{};
			blasInfo.dirty = true;

			// 1. Vertex / Index Buffer 作成
			{// vertex
				VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(sizeof(VertexData) * mesh.vertex.size(), VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
				bufferCreateInfo.usage |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VkBufferUsageFlagBits::VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
				VmaAllocationCreateInfo allocationCreateInfo{};
				allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;	// CPUで更新可能
				VK_VALIDATION(vmaCreateBuffer(m_device->GetVmaAllocator(), &bufferCreateInfo, &allocationCreateInfo, &blasInfo.vertexBuffer.buffer, &blasInfo.vertexBuffer.allocation, &blasInfo.vertexBuffer.allocationInfo));
				blasInfo.vertexBuffer.descriptor = Utility::Vulkan::CreateInfo::descriptorBufferInfo(blasInfo.vertexBuffer.buffer);
				// mapping
				void* data{};
				VK_VALIDATION(vmaMapMemory(m_device->GetVmaAllocator(), blasInfo.vertexBuffer.allocation, &data));
				memcpy(data, mesh.vertex.data(), bufferCreateInfo.size);
				vmaUnmapMemory(m_device->GetVmaAllocator(), blasInfo.vertexBuffer.allocation);
			}
			{// index
				VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(sizeof(uint32_t) * mesh.index.size(), VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
				bufferCreateInfo.usage |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VkBufferUsageFlagBits::VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
				VmaAllocationCreateInfo allocationCreateInfo{};
				allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;	// CPUで更新可能
				VK_VALIDATION(vmaCreateBuffer(m_device->GetVmaAllocator(), &bufferCreateInfo, &allocationCreateInfo, &blasInfo.indexBuffer.buffer, &blasInfo.indexBuffer.allocation, &blasInfo.indexBuffer.allocationInfo));
				blasInfo.indexBuffer.descriptor = Utility::Vulkan::CreateInfo::descriptorBufferInfo(blasInfo.indexBuffer.buffer);

				// mapping
				void* data{};
				VK_VALIDATION(vmaMapMemory(m_device->GetVmaAllocator(), blasInfo.indexBuffer.allocation, &data));
				memcpy(data, mesh.index.data(), bufferCreateInfo.size);
				vmaUnmapMemory(m_device->GetVmaAllocator(), blasInfo.indexBuffer.allocation);
			}
			{// transform
				// MeshTransform (固定)
				VkTransformMatrixKHR transformMatrix = {
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f };
				// transformBuffer 作成
				vmaTools::CreateBuffer(
					*m_device,
					m_device->GetVmaAllocator(),
					sizeof(VkTransformMatrixKHR),
					VkBufferUsageFlagBits::VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
					VkBufferUsageFlagBits::VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
					blasInfo.transformBuffer.buffer,
					blasInfo.transformBuffer.allocation,
					blasInfo.transformBuffer.allocationInfo,
					blasInfo.transformBuffer.descriptor);
			}

			// 2. VkDeviceAddress 取得
			VkDeviceOrHostAddressConstKHR vertexAddress{};
			blasInfo.vertexAddress = vertexAddress.deviceAddress = m_device->GetBufferDeviceAddress(blasInfo.vertexBuffer.buffer);
			VkDeviceOrHostAddressConstKHR indexAddress{};
			blasInfo.indexAddress = indexAddress.deviceAddress = m_device->GetBufferDeviceAddress(blasInfo.indexBuffer.buffer);
			VkDeviceOrHostAddressConstKHR transformAddress{};
			blasInfo.transformAddress = transformAddress.deviceAddress = m_device->GetBufferDeviceAddress(blasInfo.transformBuffer.buffer);

			// 3. BLAS Geometry 情報構築
			blasInfo.geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
			blasInfo.geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
			blasInfo.geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
			blasInfo.geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
			blasInfo.geometry.geometry.triangles.vertexData = vertexAddress;
			blasInfo.geometry.geometry.triangles.maxVertex = static_cast<uint32_t>(mesh.vertex.size());
			blasInfo.geometry.geometry.triangles.vertexStride = sizeof(VertexData);
			blasInfo.geometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
			blasInfo.geometry.geometry.triangles.indexData = indexAddress;
			blasInfo.geometry.geometry.triangles.transformData = transformAddress;

			// ASのビルドジオメトリ情報
			VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo{};
			buildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
			buildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			buildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
			buildGeometryInfo.geometryCount = 1;	// ジオメトリ数(今は1つだけ)
			buildGeometryInfo.pGeometries = &blasInfo.geometry;

			// プリミティブ数
			const uint32_t primitiveCount = static_cast<uint32_t>(mesh.index.size() / 3);

			// ASのビルド範囲
			blasInfo.buildRange.firstVertex = 0;
			blasInfo.buildRange.primitiveOffset = 0;
			blasInfo.buildRange.primitiveCount = primitiveCount;
			blasInfo.buildRange.transformOffset = 0;

			// ASのビルドサイズ
			blasInfo.buildSize.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
			m_vkGetAccelerationStructureBuildSizesKHR(
				*m_device,
				VkAccelerationStructureBuildTypeKHR::VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
				&buildGeometryInfo,
				&primitiveCount,
				&blasInfo.buildSize);

			// BLAS Bufferの作成
			vmaTools::CreateASBuffer(
				*m_device,
				m_device->GetVmaAllocator(),
				blasInfo.buildSize.accelerationStructureSize,
				blasInfo.blas.buffer.buffer,
				blasInfo.blas.buffer.allocation,
				blasInfo.blas.buffer.allocationInfo,
				blasInfo.blas.buffer.descriptor);

			// 4. BLAS登録
			m_pendingBLASBuild.push_back(blasInfo);

			// 5. Dirty フラグ更新
			m_blasDirty = true;
		}
	}

	void AccelerationStructureManager::OnAddObject(const MObject_ptr& object)
	{
		TLASInstanceInfo info{};
		info.objectID = object->GetObjectID();
		//info.blasID = blasID;	//todo
		info.transform = { object->GetPos(), object->GetRot(), object->GetScale(), glm::vec4(0.0f) };
		info.mask = 0xFF;
		info.hitGroupOffset = 0;
		info.transformDirty = true;

		m_instances.push_back(info);
		m_tlasDirty = true;
	}

	void AccelerationStructureManager::NewScene()
	{
		m_instances.clear();
		destroyTLAS();
		m_tlasDirty = true;
	}

	void AccelerationStructureManager::Process()
	{
		if (m_pendingBLASBuild.size() > 0)
		{
			buildBLAS();
		}
	}

	void AccelerationStructureManager::buildBLAS()
	{
		uint64_t maxBuildScratchSize = 0;
		for (const BLASInfo& blasInfo : m_pendingBLASBuild)
		{
			maxBuildScratchSize = std::max(maxBuildScratchSize, static_cast<uint64_t>(blasInfo.buildSize.buildScratchSize));
		}

		// Scratch Bufferの作成
		ScratchBuffer scratchBuffer{};
		vmaTools::CreateScratchBuffer(
			*m_device,
			m_device->GetVmaAllocator(),
			maxBuildScratchSize,
			scratchBuffer.buffer.buffer,
			scratchBuffer.buffer.allocation,
			scratchBuffer.buffer.allocationInfo,
			scratchBuffer.buffer.descriptor);
		scratchBuffer.deviceAddress = m_device->GetBufferDeviceAddress(scratchBuffer.buffer.buffer);

		// CommandBufferの準備
		VkCommandBuffer commandBuffer = m_device->GetGraphicsQueue()->AllocateSingleUseCommandBuffer(*m_device);
		VkCommandBufferBeginInfo commandBufferBeginInfo = Utility::Vulkan::CreateInfo::commandBufferBeginInfo();
		VK_VALIDATION(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

		for (BLASInfo& blasInfo : m_pendingBLASBuild)
		{
			// ASの作成
			VkAccelerationStructureCreateInfoKHR asCI{};
			asCI.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
			asCI.buffer = blasInfo.blas.buffer.buffer;
			asCI.size = blasInfo.buildSize.accelerationStructureSize;
			asCI.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			VK_VALIDATION(m_vkCreateAccelerationStructureKHR(*m_device, &asCI, m_device->GetAllocationCallbacks(), &blasInfo.blas.handle));

			// ASのビルドジオメトリ情報
			VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo{};
			buildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
			buildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			buildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
			buildGeometryInfo.geometryCount = 1;	// ジオメトリ数(今は1つだけ)
			buildGeometryInfo.pGeometries = &blasInfo.geometry;
			buildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
			buildGeometryInfo.dstAccelerationStructure = blasInfo.blas.handle;
			buildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

			// 1つのみ
			const VkAccelerationStructureBuildRangeInfoKHR* pAsBuildRangeInfoKHR = &blasInfo.buildRange;

			// ビルド
			m_vkCmdBuildAccelerationStructuresKHR(
				commandBuffer,
				1,
				&buildGeometryInfo,
				&pAsBuildRangeInfoKHR);
		}

		// 書き込む保証 (FenceではできないGPU->GPUの書き込みバリア)
		VkMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
		barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
			VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR |
			VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
			0,
			1, &barrier,
			0, nullptr,
			0, nullptr);

		VK_VALIDATION(vkEndCommandBuffer(commandBuffer));

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
		for (BLASInfo& blasInfo : m_pendingBLASBuild)
		{
			VkAccelerationStructureDeviceAddressInfoKHR asDeviceAddressInfo{};
			asDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
			asDeviceAddressInfo.accelerationStructure = blasInfo.blas.handle;
			blasInfo.blas.deviceAddress = m_vkGetAccelerationStructureDeviceAddressKHR(*m_device, &asDeviceAddressInfo);
		}

		// ScratchBufferの破棄
		vmaDestroyBuffer(m_device->GetVmaAllocator(), scratchBuffer.buffer.buffer, scratchBuffer.buffer.allocation);
	}

	void AccelerationStructureManager::buildTLAS()
	{

	}

	void AccelerationStructureManager::destroyTLAS()
	{
		if (m_tlas.tlas.handle != VK_NULL_HANDLE)
		{
			m_vkDestroyAccelerationStructureKHR(*m_device, m_tlas.tlas.handle, m_device->GetAllocationCallbacks());
			m_tlas.tlas.handle = VK_NULL_HANDLE;
		}
		if (m_tlas.tlas.buffer.buffer != VK_NULL_HANDLE)
		{
			vmaDestroyBuffer(m_device->GetVmaAllocator(), m_tlas.tlas.buffer.buffer, m_tlas.tlas.buffer.allocation);
		}
		if (m_tlas.instanceBuffer.buffer != VK_NULL_HANDLE)
		{
			vmaDestroyBuffer(m_device->GetVmaAllocator(), m_tlas.instanceBuffer.buffer, m_tlas.instanceBuffer.allocation);
		}
		m_tlas.tlas.deviceAddress = 0;
	}
}
