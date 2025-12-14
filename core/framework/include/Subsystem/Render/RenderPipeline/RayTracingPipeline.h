// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"

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
		RayTracingPipeline(const RenderDevice_ptr& device,
			const SceneInfoDescriptorSet_ptr& sceneInfoDescriptorSet,
			const ObjectInfoDescriptorSet_ptr& objectInfoDescriptorSet,
			const MeshInfoDescriptorSet_ptr& meshInfoDescriptorSet,
			const TextureDescriptorSet_ptr& textureDescriptorSet
		) :
			RenderPipelineBase(device, sceneInfoDescriptorSet, objectInfoDescriptorSet, meshInfoDescriptorSet, textureDescriptorSet),
			//pushConstant({}),
			m_vertexBuffer({}),
			m_indexBuffer({}) {
		}
		~RayTracingPipeline();

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer);

	private:
		//struct {
		//	glm::ivec2 atlasSize;
		//	glm::ivec2 offset;
		//	glm::ivec2 size;
		//}pushConstant;

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

		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;

		void createShaderBindingTable();
		void createSBTBuffer(VkDeviceSize size, Buffer* sbtBuffer, const void* data);

		Buffer m_vertexBuffer;
		Buffer m_indexBuffer;
	};
	TYPEDEF_SHARED_PTR_ARGS(RayTracingPipeline);
}