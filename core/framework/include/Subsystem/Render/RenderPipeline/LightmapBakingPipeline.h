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
	class LightmapBakingPipeline : public RenderPipelineBase
	{
	public:
		LightmapBakingPipeline(const RenderDevice_ptr& device, const RenderDescriptors_ptr& renderDescriptors) :
			RenderPipelineBase(device, renderDescriptors),
			pushConstant({}),
			m_vertexBuffer({}),
			m_indexBuffer({}),
			m_isBaking(false),
			lightmapAllocateTools({}) {
		}
		~LightmapBakingPipeline();

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer);
		bool NextObject(const RenderResources_ptr& resources, const MObject_ptr& object);
		void Bake();
		bool IsBaking() const { return m_isBaking; }
		void OutputLightmap(const RenderResources_ptr& resources, const uint32_t frameIndex);

	private:
		struct PushConstant {
			glm::ivec2 atlasSize;
			glm::ivec2 offset;
			glm::ivec2 size;
		}pushConstant;

		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;

		std::vector<Buffer> m_vertexBuffer;
		std::vector<Buffer> m_indexBuffer;
		std::vector<glm::ivec2> m_atlasSize;

		bool m_isBaking = false;

		// Lightmap詰め込み用
		bool allocateLightmapAtlas(const glm::ivec2& size, glm::ivec2& offset);
		struct {
			glm::ivec2 current;
			uint32_t bottom;
		}lightmapAllocateTools;

	};
	TYPEDEF_SHARED_PTR_ARGS(LightmapBakingPipeline);
}