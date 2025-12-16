// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "AccelerationStructure.h"

namespace MyosotisFW
{
	class MObject;
	TYPEDEF_SHARED_PTR_FWD(MObject);
}

namespace MyosotisFW::System::Render
{
	class RayTracingPipeline : public RenderPipelineBase
	{
	public:
		RayTracingPipeline(const RenderDevice_ptr& device, const RenderDescriptors_ptr& renderDescriptors);
		~RayTracingPipeline();

		void Initialize(const RenderResources_ptr& resources);
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer);

	private:
		struct {
			Buffer sbtBuffer;
			VkStridedDeviceAddressRegionKHR region;
		} m_raygenSBTBuffer;
		struct {
			Buffer sbtBuffer;
			VkStridedDeviceAddressRegionKHR region;
		} m_missSBTBuffer;
		struct {
			Buffer sbtBuffer;
			VkStridedDeviceAddressRegionKHR region;
		} m_hitSBTBuffer;

		AccelerationStructure m_blas;
		AccelerationStructure m_tlas;

		void prepareRenderPipeline(const RenderResources_ptr& resources);

		void createShaderBindingTable();
		void createSBTBuffer(Buffer& buffer, const uint32_t handleSize, const uint32_t handleCount);

		void createBLAS();
		void createTLAS();

		Buffer m_vertexBuffer;
		Buffer m_indexBuffer;
		Buffer m_transform;

		PFN_vkGetRayTracingShaderGroupHandlesKHR		m_vkGetRayTracingShaderGroupHandlesKHR;
		PFN_vkCreateAccelerationStructureKHR			m_vkCreateAccelerationStructureKHR;
		PFN_vkCmdBuildAccelerationStructuresKHR			m_vkCmdBuildAccelerationStructuresKHR;
		PFN_vkGetAccelerationStructureDeviceAddressKHR	m_vkGetAccelerationStructureDeviceAddressKHR;
		PFN_vkGetAccelerationStructureBuildSizesKHR		m_vkGetAccelerationStructureBuildSizesKHR;
		PFN_vkCmdTraceRaysKHR							m_vkCmdTraceRaysKHR;
		PFN_vkCreateRayTracingPipelinesKHR				m_vkCreateRayTracingPipelinesKHR;
		PFN_vkDestroyAccelerationStructureKHR			m_vkDestroyAccelerationStructureKHR;
	};
	TYPEDEF_SHARED_PTR_ARGS(RayTracingPipeline);
}