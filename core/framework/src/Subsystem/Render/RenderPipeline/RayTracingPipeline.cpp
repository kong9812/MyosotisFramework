// Copyright (c) 2025 kong9812
#include "RayTracingPipeline.h"
#include <array>
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
	RayTracingPipeline::RayTracingPipeline(const RenderDevice_ptr& device, const RenderDescriptors_ptr& renderDescriptors) :
		RenderPipelineBase(device, renderDescriptors),
		pushConstant(),
		m_raygenSBTBuffer(),
		m_missSBTBuffer(),
		m_hitSBTBuffer(),
		m_vkGetRayTracingShaderGroupHandlesKHR(VK_NULL_HANDLE),
		m_vkCreateAccelerationStructureKHR(VK_NULL_HANDLE),
		m_vkCmdBuildAccelerationStructuresKHR(VK_NULL_HANDLE),
		m_vkGetAccelerationStructureDeviceAddressKHR(VK_NULL_HANDLE),
		m_vkGetAccelerationStructureBuildSizesKHR(VK_NULL_HANDLE),
		m_vkCmdTraceRaysKHR(VK_NULL_HANDLE),
		m_vkCreateRayTracingPipelinesKHR(VK_NULL_HANDLE)
	{
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

	RayTracingPipeline::~RayTracingPipeline()
	{
		vkDestroyPipeline(*m_device, m_pipeline, m_device->GetAllocationCallbacks());
		vkDestroyPipelineLayout(*m_device, m_pipelineLayout, m_device->GetAllocationCallbacks());

		vmaDestroyBuffer(m_device->GetVmaAllocator(), m_raygenSBTBuffer.sbtBuffer.buffer, m_raygenSBTBuffer.sbtBuffer.allocation);
		vmaDestroyBuffer(m_device->GetVmaAllocator(), m_missSBTBuffer.sbtBuffer.buffer, m_missSBTBuffer.sbtBuffer.allocation);
		vmaDestroyBuffer(m_device->GetVmaAllocator(), m_hitSBTBuffer.sbtBuffer.buffer, m_hitSBTBuffer.sbtBuffer.allocation);
	}

	void RayTracingPipeline::Initialize(const RenderResources_ptr& resources)
	{
		prepareRenderPipeline(resources);

		{// RayTracingRenderTarget -> VK_IMAGE_LAYOUT_GENERAL
			std::array<VkImageMemoryBarrier, AppInfo::g_maxInFlightFrameCount> barrier{};
			for (uint32_t i = 0; i < AppInfo::g_maxInFlightFrameCount; i++)
			{
				const Image& rayTracingRenderTarget = resources->GetRayTracingRenderTarget(i);

				// [storage image] ray tracing render target
				VkDescriptorImageInfo descriptorImageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(VK_NULL_HANDLE, rayTracingRenderTarget.view, VkImageLayout::VK_IMAGE_LAYOUT_GENERAL);
				pushConstant[i].storeImageID = m_renderDescriptors->GetTextureDescriptorSet()->AddImage(TextureDescriptorSet::DescriptorBindingIndex::StorageImage, descriptorImageInfo);

				barrier[i].sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier[i].srcAccessMask = VkAccessFlagBits::VK_ACCESS_NONE;
				barrier[i].dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_WRITE_BIT;
				barrier[i].oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
				barrier[i].newLayout = VkImageLayout::VK_IMAGE_LAYOUT_GENERAL;
				barrier[i].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier[i].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier[i].image = rayTracingRenderTarget.image;
				barrier[i].subresourceRange = Utility::Vulkan::CreateInfo::defaultImageSubresourceRange(VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT);
			}

			VkFence fence = VK_NULL_HANDLE;
			VkFenceCreateInfo fenceCreateInfo = Utility::Vulkan::CreateInfo::fenceCreateInfo();
			VK_VALIDATION(vkCreateFence(*m_device, &fenceCreateInfo, m_device->GetAllocationCallbacks(), &fence));

			VkCommandBuffer commandBuffer = m_device->GetGraphicsQueue()->AllocateSingleUseCommandBuffer(*m_device);

			VkCommandBufferBeginInfo commandBufferBeginInfo = Utility::Vulkan::CreateInfo::commandBufferBeginInfo();
			VK_VALIDATION(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));
			vkCmdPipelineBarrier(commandBuffer,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
				0, 0, nullptr, 0, nullptr, static_cast<uint32_t>(barrier.size()), barrier.data());
			VK_VALIDATION(vkEndCommandBuffer(commandBuffer));

			VkSubmitInfo submitInfo = Utility::Vulkan::CreateInfo::submitInfo();
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			m_device->GetGraphicsQueue()->Submit(submitInfo, fence);
			VK_VALIDATION(vkWaitForFences(*m_device, 1, &fence, VK_TRUE, UINT64_MAX));

			m_device->GetGraphicsQueue()->FreeSingleUseCommandBuffer(*m_device, commandBuffer);
			vkDestroyFence(*m_device, fence, m_device->GetAllocationCallbacks());
		}
	}

	void RayTracingPipeline::BindCommandBuffer(const VkCommandBuffer& commandBuffer, const uint32_t frameIndex, const glm::ivec2& screenSize)
	{
		vkCmdBindPipeline(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_pipeline);
		std::vector<VkDescriptorSet> descriptorSets = m_renderDescriptors->GetDescriptorSet();
		vkCmdBindDescriptorSets(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_pipelineLayout, 0,
			static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, NULL);
		vkCmdPushConstants(commandBuffer, m_pipelineLayout,
			VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_KHR,
			0, static_cast<uint32_t>(sizeof(PushConstant)), &pushConstant[frameIndex]);

		VkStridedDeviceAddressRegionKHR callableRegion{};
		callableRegion.deviceAddress = 0;
		callableRegion.size = 0;
		callableRegion.stride = 0;
		m_vkCmdTraceRaysKHR(commandBuffer,
			&m_raygenSBTBuffer.region,
			&m_missSBTBuffer.region,
			&m_hitSBTBuffer.region,
			&callableRegion,
			screenSize.x,
			screenSize.y,
			1);
	}

	void RayTracingPipeline::Resize(const RenderResources_ptr& resources)
	{
		{// RayTracingRenderTarget -> VK_IMAGE_LAYOUT_GENERAL
			std::array<VkImageMemoryBarrier, AppInfo::g_maxInFlightFrameCount> barrier{};
			for (uint32_t i = 0; i < AppInfo::g_maxInFlightFrameCount; i++)
			{
				const Image& rayTracingRenderTarget = resources->GetRayTracingRenderTarget(i);

				// [storage image] ray tracing render target
				VkDescriptorImageInfo descriptorImageInfo = Utility::Vulkan::CreateInfo::descriptorImageInfo(VK_NULL_HANDLE, rayTracingRenderTarget.view, VkImageLayout::VK_IMAGE_LAYOUT_GENERAL);
				m_renderDescriptors->GetTextureDescriptorSet()->UpdateImage(pushConstant[i].storeImageID, TextureDescriptorSet::DescriptorBindingIndex::StorageImage, descriptorImageInfo);

				barrier[i].sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier[i].srcAccessMask = VkAccessFlagBits::VK_ACCESS_NONE;
				barrier[i].dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_WRITE_BIT;
				barrier[i].oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
				barrier[i].newLayout = VkImageLayout::VK_IMAGE_LAYOUT_GENERAL;
				barrier[i].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier[i].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier[i].image = rayTracingRenderTarget.image;
				barrier[i].subresourceRange = Utility::Vulkan::CreateInfo::defaultImageSubresourceRange(VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT);
			}

			VkFence fence = VK_NULL_HANDLE;
			VkFenceCreateInfo fenceCreateInfo = Utility::Vulkan::CreateInfo::fenceCreateInfo();
			VK_VALIDATION(vkCreateFence(*m_device, &fenceCreateInfo, m_device->GetAllocationCallbacks(), &fence));

			VkCommandBuffer commandBuffer = m_device->GetGraphicsQueue()->AllocateSingleUseCommandBuffer(*m_device);

			VkCommandBufferBeginInfo commandBufferBeginInfo = Utility::Vulkan::CreateInfo::commandBufferBeginInfo();
			VK_VALIDATION(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));
			vkCmdPipelineBarrier(commandBuffer,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
				0, 0, nullptr, 0, nullptr, static_cast<uint32_t>(barrier.size()), barrier.data());
			VK_VALIDATION(vkEndCommandBuffer(commandBuffer));

			VkSubmitInfo submitInfo = Utility::Vulkan::CreateInfo::submitInfo();
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			m_device->GetGraphicsQueue()->Submit(submitInfo, fence);
			VK_VALIDATION(vkWaitForFences(*m_device, 1, &fence, VK_TRUE, UINT64_MAX));

			m_device->GetGraphicsQueue()->FreeSingleUseCommandBuffer(*m_device, commandBuffer);
			vkDestroyFence(*m_device, fence, m_device->GetAllocationCallbacks());
		}
	}

	void RayTracingPipeline::prepareRenderPipeline(const RenderResources_ptr& resources)
	{
		// push constant
		std::vector<VkPushConstantRange> pushConstantRange = {
			// RAYGEN
			Utility::Vulkan::CreateInfo::pushConstantRange(VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_KHR,
				0,
				static_cast<uint32_t>(sizeof(PushConstant))),
		};

		// [pipeline]layout
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts = m_renderDescriptors->GetDescriptorSetLayout();
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = Utility::Vulkan::CreateInfo::pipelineLayoutCreateInfo(descriptorSetLayouts);
		pipelineLayoutCreateInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRange.size());
		pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRange.data();
		VK_VALIDATION(vkCreatePipelineLayout(*m_device, &pipelineLayoutCreateInfo, m_device->GetAllocationCallbacks(), &m_pipelineLayout));

		// shader stages
		std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo{
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_KHR, resources->GetShaderModules("RayTracing.rgen.spv")),
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_MISS_BIT_KHR, resources->GetShaderModules("RayTracing.rmiss.spv")),
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_MISS_BIT_KHR, resources->GetShaderModules("Shadow.rmiss.spv")),
			Utility::Vulkan::CreateInfo::pipelineShaderStageCreateInfo(VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, resources->GetShaderModules("RayTracing.rchit.spv")),
		};
		// shader groups
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> rayTracingShaderGroupCreateInfoKHR{
			Utility::Vulkan::CreateInfo::rayTracingShaderGroupCreateInfo(VkRayTracingShaderGroupTypeKHR::VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR, 0),
			Utility::Vulkan::CreateInfo::rayTracingShaderGroupCreateInfo(VkRayTracingShaderGroupTypeKHR::VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR, 1),
			Utility::Vulkan::CreateInfo::rayTracingShaderGroupCreateInfo(VkRayTracingShaderGroupTypeKHR::VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR, 2),
			Utility::Vulkan::CreateInfo::rayTracingShaderGroupCreateInfo(VkRayTracingShaderGroupTypeKHR::VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR, VK_SHADER_UNUSED_KHR, 3),
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
		VK_VALIDATION(m_vkCreateRayTracingPipelinesKHR(*m_device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineCreateInfoKHR, m_device->GetAllocationCallbacks(), &m_pipeline));

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

		uint32_t raygenHandleCount = 1;
		uint32_t missHandleCount = 2;	// 通常用 + 影用
		uint32_t chitHandleCount = 1;

		const uint32_t groupCount = raygenHandleCount + missHandleCount + chitHandleCount;
		const uint32_t sbtSize = groupCount * handleSizeAligned;

		// 全シェーダーグループハンドルの取得
		std::vector<uint8_t> shaderHandleStorage(sbtSize);
		VK_VALIDATION(m_vkGetRayTracingShaderGroupHandlesKHR(
			*m_device,
			m_pipeline,
			0,				// 最初のシェーダーグループインデックス
			groupCount,		// 取得するシェーダーグループ数
			sbtSize,		// バッファサイズ
			shaderHandleStorage.data()));

		// raygen SBTの作成
		createSBTBuffer(m_raygenSBTBuffer.sbtBuffer, handleSize, raygenHandleCount);
		m_raygenSBTBuffer.region.deviceAddress = m_device->GetBufferDeviceAddress(m_raygenSBTBuffer.sbtBuffer.buffer);
		m_raygenSBTBuffer.region.stride = handleSizeAligned;
		m_raygenSBTBuffer.region.size = handleSizeAligned * raygenHandleCount;
		// miss SBTの作成
		createSBTBuffer(m_missSBTBuffer.sbtBuffer, handleSize, missHandleCount);
		m_missSBTBuffer.region.deviceAddress = m_device->GetBufferDeviceAddress(m_missSBTBuffer.sbtBuffer.buffer);
		m_missSBTBuffer.region.stride = handleSizeAligned;
		m_missSBTBuffer.region.size = handleSizeAligned * missHandleCount;
		// chit SBTの作成
		createSBTBuffer(m_hitSBTBuffer.sbtBuffer, handleSize, chitHandleCount);
		m_hitSBTBuffer.region.deviceAddress = m_device->GetBufferDeviceAddress(m_hitSBTBuffer.sbtBuffer.buffer);
		m_hitSBTBuffer.region.stride = handleSizeAligned;
		m_hitSBTBuffer.region.size = handleSizeAligned * chitHandleCount;

		// copy handle
		// RayGen (Group 0)
		vmaTools::MemcpyBufferData(m_device->GetVmaAllocator(), m_raygenSBTBuffer.sbtBuffer, shaderHandleStorage.data() + (0 * handleSize), handleSize);
		// Miss (Group 1 & 2)
		// 1つ目の Miss
		vmaTools::MemcpyBufferData(m_device->GetVmaAllocator(), m_missSBTBuffer.sbtBuffer, shaderHandleStorage.data() + (1 * handleSize), handleSize, 0);
		// 2つ目の Miss (影用)
		vmaTools::MemcpyBufferData(m_device->GetVmaAllocator(), m_missSBTBuffer.sbtBuffer, shaderHandleStorage.data() + (2 * handleSize), handleSize, handleSizeAligned); // オフセットを追加
		// Hit (Group 3)
		vmaTools::MemcpyBufferData(m_device->GetVmaAllocator(), m_hitSBTBuffer.sbtBuffer, shaderHandleStorage.data() + (3 * handleSize), handleSize);
	}

	void RayTracingPipeline::createSBTBuffer(Buffer& buffer, const uint32_t handleSize, const uint32_t handleCount)
	{
		const VkDeviceSize sbtSize = handleSize * handleCount;
		// SBTバッファの作成
		buffer = vmaTools::CreateBuffer(
			m_device->GetVmaAllocator(), sbtSize,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR |
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU,
			VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT);
	}
}