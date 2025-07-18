// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"

namespace MyosotisFW::System::Render
{
	class BindlessResourcesRenderPipeline : public RenderPipelineBase
	{
	public:
		BindlessResourcesRenderPipeline(const RenderDevice_ptr& device) :
			RenderPipelineBase(device),
			m_shaderBase({}) {
		}
		~BindlessResourcesRenderPipeline();

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer);
		void CreateShaderObject(const glm::vec2& screenSize);
		void UpdateDescriptors();
		void Resize(const RenderResources_ptr& resources) override;

	protected:
		void prepareDescriptors() override;
		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;

		ShaderBase m_shaderBase;
		Image m_image;

	private:
		struct {
			uint32_t textureId;
			uint32_t bufferId;
			glm::vec2 screenSize;
		}m_pushConstant;
	};
	TYPEDEF_SHARED_PTR_ARGS(BindlessResourcesRenderPipeline);
}