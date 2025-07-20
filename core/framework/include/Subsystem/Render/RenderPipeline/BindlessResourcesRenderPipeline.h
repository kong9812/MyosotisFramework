// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"

namespace MyosotisFW::System::Render
{
	class BindlessResourcesRenderPipeline : public RenderPipelineBase
	{
	public:
		BindlessResourcesRenderPipeline(const RenderDevice_ptr& device, const RenderDescriptors_ptr& descriptors) :
			RenderPipelineBase(device, descriptors),
			m_image({}),
			m_shaderBase({}),
			m_pushConstant({}) {
		}
		~BindlessResourcesRenderPipeline();

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer);
		void CreateShaderObject(const glm::vec2& screenSize);
		void UpdateDescriptors();

	protected:
		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;

		ShaderBase m_shaderBase;
		Image m_image;

	private:
		struct {
			uint32_t textureId;
			uint32_t empty;
			glm::vec2 screenSize;
		}m_pushConstant;
	};
	TYPEDEF_SHARED_PTR_ARGS(BindlessResourcesRenderPipeline);
}