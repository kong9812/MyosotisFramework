// Copyright (c) 2025 kong9812
#include "AccelerationStructureManager.h"
#include <algorithm>
#include "RenderDevice.h"
#include "RenderQueue.h"
#include "MObject.h"
#include "iglm.h"
#include "RenderDescriptors.h"
#include "RenderResources.h"
#include "TLASInstanceInfo.h"

namespace {
	VkTransformMatrixKHR ToVkTransformMatrixKHR(const glm::mat4& mat)
	{
		VkTransformMatrixKHR vkMat = {
			mat[0][0],mat[1][0],mat[2][0],mat[3][0],
			mat[0][1],mat[1][1],mat[2][1],mat[3][1],
			mat[0][2],mat[1][2],mat[2][2],mat[3][2],
		};
		return vkMat;
	}
}

namespace MyosotisFW::System::Render
{
	AccelerationStructureManager::AccelerationStructureManager(
		const RenderDevice_ptr& device,
		const RenderDescriptors_ptr& renderDescriptors,
		const RenderResources_ptr& renderResources) :
		m_device(device),
		m_renderDescriptors(renderDescriptors),
		m_renderResources(renderResources),
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
			//vmaDestroyBuffer(m_device->GetVmaAllocator(), blasInfo.vertexBuffer.buffer, blasInfo.vertexBuffer.allocation);
			//vmaDestroyBuffer(m_device->GetVmaAllocator(), blasInfo.indexBuffer.buffer, blasInfo.indexBuffer.allocation);
			//vmaDestroyBuffer(m_device->GetVmaAllocator(), blasInfo.transformBuffer.buffer, blasInfo.transformBuffer.allocation);

			vmaDestroyBuffer(m_device->GetVmaAllocator(), blasInfo.blas.buffer.buffer, blasInfo.blas.buffer.allocation);

			m_vkDestroyAccelerationStructureKHR(*m_device, blasInfo.blas.handle, m_device->GetAllocationCallbacks());
		}
		for (const BLASInfo& blasInfo : m_blas)
		{
			//vmaDestroyBuffer(m_device->GetVmaAllocator(), blasInfo.vertexBuffer.buffer, blasInfo.vertexBuffer.allocation);
			//vmaDestroyBuffer(m_device->GetVmaAllocator(), blasInfo.indexBuffer.buffer, blasInfo.indexBuffer.allocation);
			//vmaDestroyBuffer(m_device->GetVmaAllocator(), blasInfo.transformBuffer.buffer, blasInfo.transformBuffer.allocation);

			vmaDestroyBuffer(m_device->GetVmaAllocator(), blasInfo.blas.buffer.buffer, blasInfo.blas.buffer.allocation);

			m_vkDestroyAccelerationStructureKHR(*m_device, blasInfo.blas.handle, m_device->GetAllocationCallbacks());
		}

		vmaDestroyBuffer(m_device->GetVmaAllocator(), m_tlas.tlas.buffer.buffer, m_tlas.tlas.buffer.allocation);
		vmaDestroyBuffer(m_device->GetVmaAllocator(), m_tlas.instanceBuffer.buffer, m_tlas.instanceBuffer.allocation);
		m_vkDestroyAccelerationStructureKHR(*m_device, m_tlas.tlas.handle, m_device->GetAllocationCallbacks());
	}

	void AccelerationStructureManager::OnLoadedMesh(MeshesHandle& meshes)
	{
		for (MeshHandle& mesh : meshes)
		{
			std::shared_ptr<const Mesh> tmp = mesh.lock();

			// BLAS Info
			BLASInfo blasInfo{};
			blasInfo.dirty = true;

			// 1. Vertex / Index Buffer
			blasInfo.vertexBuffer = tmp->vertexBuffer;
			blasInfo.indexBuffer = tmp->indexBuffer;

			// 2. VkDeviceAddress 取得
			VkDeviceOrHostAddressConstKHR vertexAddress{};
			blasInfo.vertexAddress = vertexAddress.deviceAddress = m_device->GetBufferDeviceAddress(blasInfo.vertexBuffer.buffer);
			VkDeviceOrHostAddressConstKHR indexAddress{};
			blasInfo.indexAddress = indexAddress.deviceAddress = m_device->GetBufferDeviceAddress(blasInfo.indexBuffer.buffer);

			// 3. BLAS Geometry 情報構築
			blasInfo.geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
			blasInfo.geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
			blasInfo.geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
			blasInfo.geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
			blasInfo.geometry.geometry.triangles.vertexData = vertexAddress;
			blasInfo.geometry.geometry.triangles.maxVertex = static_cast<uint32_t>(tmp->vertex.size() - 1);
			blasInfo.geometry.geometry.triangles.vertexStride = sizeof(VertexData);
			blasInfo.geometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
			blasInfo.geometry.geometry.triangles.indexData = indexAddress;
			//blasInfo.geometry.geometry.triangles.transformData = transformAddress;

			// ASのビルドジオメトリ情報
			VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo{};
			buildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
			buildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			buildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
			buildGeometryInfo.geometryCount = 1;	// ジオメトリ数(今は1つだけ)
			buildGeometryInfo.pGeometries = &blasInfo.geometry;

			// プリミティブ数
			const uint32_t primitiveCount = static_cast<uint32_t>(tmp->index.size() / 3);

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
			blasInfo.blas.buffer = vmaTools::CreateASBuffer(m_device->GetVmaAllocator(), blasInfo.buildSize.accelerationStructureSize);

			// 4. BLAS登録
			tmp->blasID = m_pendingBLASBuild.size();
			m_pendingBLASBuild.push_back(blasInfo);

			// 5. Dirty フラグ更新
			m_blasDirty = true;
		}
	}

	void AccelerationStructureManager::OnAddObject(const MObject_ptr& object)
	{
		TLASInstance_ptr info = std::make_shared<TLASInstance>();
		info->objectID = object->GetObjectID();
		info->active = false;
		//info.blasID = blasID;	//todo
		//info.transform = { object->GetPos(), object->GetRot(), object->GetScale(), glm::vec4(0.0f) };
		info->mask = 0xFF;
		info->hitGroupOffset = 0;
		info->transformDirty = true;
		m_instances.push_back(info);
		object->SetTLASInstance(info);
		m_tlasDirty = true;
	}

	void AccelerationStructureManager::RebuildTLAS()
	{
		destroyTLAS();
		m_tlasDirty = true;
	}

	void AccelerationStructureManager::Process()
	{
		if (m_blasDirty)
		{
			buildBLAS();
		}
		if (m_tlasDirty)
		{
			buildTLAS();
		}
	}

	void AccelerationStructureManager::buildBLAS()
	{
		uint64_t totalScratchSize = 0;
		for (const BLASInfo& blasInfo : m_pendingBLASBuild)
		{
			totalScratchSize += blasInfo.buildSize.buildScratchSize;
		}

		// Scratch Bufferの作成
		std::vector<ScratchBuffer> scratchBuffer(m_pendingBLASBuild.size());
		uint64_t scratchBufferOffset = 0;

		// CommandBufferの準備
		VkCommandBuffer commandBuffer = m_device->GetGraphicsQueue()->AllocateSingleUseCommandBuffer(*m_device);
		VkCommandBufferBeginInfo commandBufferBeginInfo = Utility::Vulkan::CreateInfo::commandBufferBeginInfo();
		VK_VALIDATION(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

		uint64_t scratchBufferIndex = 0;
		for (BLASInfo& blasInfo : m_pendingBLASBuild)
		{
			scratchBuffer[scratchBufferIndex].buffer = vmaTools::CreateScratchBuffer(m_device->GetVmaAllocator(), totalScratchSize);
			scratchBuffer[scratchBufferIndex].deviceAddress = m_device->GetBufferDeviceAddress(scratchBuffer[scratchBufferIndex].buffer.buffer);

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
			//buildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress + scratchBufferOffset;
			//scratchBufferOffset += blasInfo.buildSize.buildScratchSize;
			buildGeometryInfo.scratchData.deviceAddress = scratchBuffer[scratchBufferIndex].deviceAddress;

			// 1つのみ
			const VkAccelerationStructureBuildRangeInfoKHR* pAsBuildRangeInfoKHR = &blasInfo.buildRange;

			// ビルド
			m_vkCmdBuildAccelerationStructuresKHR(
				commandBuffer,
				1,
				&buildGeometryInfo,
				&pAsBuildRangeInfoKHR);

			scratchBufferIndex++;
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

			m_blas.push_back(std::move(blasInfo));
		}

		m_pendingBLASBuild.clear();

		for (ScratchBuffer& scratchBuf : scratchBuffer)
		{
			// ScratchBufferの破棄
			vmaDestroyBuffer(m_device->GetVmaAllocator(), scratchBuf.buffer.buffer, scratchBuf.buffer.allocation);
		}

		m_blasDirty = false;
	}

	void AccelerationStructureManager::buildTLAS()
	{
		if ((m_tlas.tlas.handle != VK_NULL_HANDLE) || (m_tlas.instanceBuffer.buffer != VK_NULL_HANDLE)) destroyTLAS();

		std::vector<VkAccelerationStructureInstanceKHR> asInstances{};
		for (const TLASInstance_ptr instance : m_instances)
		{
			if (!instance->active) continue;

			for (const uint32_t meshID : instance->meshID)
			{
				MeshHandle& mesh = m_renderResources->GetMeshFormID(meshID);
				std::shared_ptr<const Mesh> tmp = mesh.lock();
				uint32_t biasID = tmp->blasID;
				VkAccelerationStructureInstanceKHR asInstance{};
				asInstance.transform = ToVkTransformMatrixKHR(instance->model);
				asInstance.mask = instance->mask;
				asInstance.instanceShaderBindingTableRecordOffset = instance->hitGroupOffset;
				//asInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
				asInstance.accelerationStructureReference = m_blas[biasID].blas.deviceAddress;

				TLASInstanceInfo tlasInstanceInfo{};
				tlasInstanceInfo.meshID = meshID;
				tlasInstanceInfo.objectID = instance->objectID;

				// Custom Index
				asInstance.instanceCustomIndex = m_renderDescriptors->GetRayTracingDescriptorSet()->AddTLASInstanceInfo(tlasInstanceInfo);

				asInstances.push_back(asInstance);
			}
		}

		VkDeviceOrHostAddressConstKHR instanceDataAddress{};
		if (asInstances.size() > 0)
		{
			// AS Instance Bufferの作成
			size_t bufferSize = sizeof(VkAccelerationStructureInstanceKHR) * asInstances.size();
			m_tlas.instanceBuffer = vmaTools::CreateASInstanceBuffer(m_device->GetVmaAllocator(), bufferSize);
			vmaTools::MemcpyBufferData(m_device->GetVmaAllocator(), m_tlas.instanceBuffer, asInstances.data(), bufferSize);
			m_tlas.instanceAddress = instanceDataAddress.deviceAddress = m_device->GetBufferDeviceAddress(m_tlas.instanceBuffer.buffer);
		}
		else
		{
			m_tlas.instanceAddress = 0;
		}

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
		m_tlas.instanceCount = asInstances.size();

		// ASのビルドサイズ情報
		VkAccelerationStructureBuildSizesInfoKHR asBuildSizesInfoKHR{};
		asBuildSizesInfoKHR.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
		m_vkGetAccelerationStructureBuildSizesKHR(
			*m_device,
			VkAccelerationStructureBuildTypeKHR::VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
			&asBuildGeometryInfoKHR,
			&m_tlas.instanceCount,
			&asBuildSizesInfoKHR);

		// TLAS Bufferの作成
		m_tlas.tlas.buffer = vmaTools::CreateASBuffer(m_device->GetVmaAllocator(), asBuildSizesInfoKHR.accelerationStructureSize);

		// ASの作成
		VkAccelerationStructureCreateInfoKHR asCI{};
		asCI.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		asCI.buffer = m_tlas.tlas.buffer.buffer;
		asCI.size = asBuildSizesInfoKHR.accelerationStructureSize;
		asCI.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		VK_VALIDATION(m_vkCreateAccelerationStructureKHR(*m_device, &asCI, m_device->GetAllocationCallbacks(), &m_tlas.tlas.handle));

		// Scratch Bufferの作成
		AccelerationStructure scratchBuffer{};
		scratchBuffer.buffer = vmaTools::CreateScratchBuffer(m_device->GetVmaAllocator(), asBuildSizesInfoKHR.buildScratchSize);
		VkDeviceAddress scratchAddress = m_device->GetBufferDeviceAddress(scratchBuffer.buffer.buffer);

		// asBuildGeometryInfoKHRの情報追加
		asBuildGeometryInfoKHR.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		asBuildGeometryInfoKHR.dstAccelerationStructure = m_tlas.tlas.handle;
		asBuildGeometryInfoKHR.scratchData.deviceAddress = scratchAddress;
		asBuildGeometryInfoKHR.geometryCount = 1;
		asBuildGeometryInfoKHR.pGeometries = &asGeometryKHR;

		// ASのビルド範囲
		VkAccelerationStructureBuildRangeInfoKHR asBuildRangeInfoKHR{};
		asBuildRangeInfoKHR.firstVertex = 0;
		asBuildRangeInfoKHR.primitiveOffset = 0;
		asBuildRangeInfoKHR.primitiveCount = m_tlas.instanceCount;
		asBuildRangeInfoKHR.transformOffset = 0;
		std::vector<VkAccelerationStructureBuildRangeInfoKHR*> pAsBuildRangeInfoKHRs = { &asBuildRangeInfoKHR };

		// CommandBufferの準備
		VkCommandBuffer commandBuffer = m_device->GetGraphicsQueue()->AllocateSingleUseCommandBuffer(*m_device);
		VkCommandBufferBeginInfo commandBufferBeginInfo = Utility::Vulkan::CreateInfo::commandBufferBeginInfo();
		VK_VALIDATION(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));
		m_vkCmdBuildAccelerationStructuresKHR(
			commandBuffer,
			1,
			&asBuildGeometryInfoKHR,
			pAsBuildRangeInfoKHRs.data());

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
		VkAccelerationStructureDeviceAddressInfoKHR asDeviceAddressInfo{};
		asDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		asDeviceAddressInfo.accelerationStructure = m_tlas.tlas.handle;
		m_tlas.tlas.deviceAddress = m_vkGetAccelerationStructureDeviceAddressKHR(*m_device, &asDeviceAddressInfo);

		// ScratchBufferの破棄
		vmaDestroyBuffer(m_device->GetVmaAllocator(), scratchBuffer.buffer.buffer, scratchBuffer.buffer.allocation);

		m_tlasDirty = false;

		// SetDescriptor
		m_renderDescriptors->GetRayTracingDescriptorSet()->SetTLAS(m_tlas.tlas.handle);
	}

	void AccelerationStructureManager::destroyTLAS()
	{
		// デバイスが静止状態まで待つ
		vkDeviceWaitIdle(*m_device);

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
