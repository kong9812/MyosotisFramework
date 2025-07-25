// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"

namespace MyosotisFW::System::Render
{
	class InteriorObjectDeferredRenderPipeline : public RenderPipelineBase
	{
	public:
		InteriorObjectDeferredRenderPipeline(const RenderDevice_ptr& device, const RenderDescriptors_ptr& descriptors) :
			RenderPipelineBase(device, descriptors) {
		}
		~InteriorObjectDeferredRenderPipeline();

		// ShaderObject
		struct ShaderObject
		{
			ShaderBase shaderBase;

			Image cubeMap;

			struct {
				uint32_t objectIndex;
				uint32_t textureId;
			}pushConstant;

			struct
			{
				StandardSSBO standardSSBO;
			}SSBO;
		};

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void CreateShaderObject(ShaderObject& shaderObject);
		void UpdateDescriptors(ShaderObject& shaderObject);

	private:
		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
	};
	TYPEDEF_SHARED_PTR_ARGS(InteriorObjectDeferredRenderPipeline);
}