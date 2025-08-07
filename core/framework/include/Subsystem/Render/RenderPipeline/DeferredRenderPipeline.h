// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"

namespace MyosotisFW::System::Render
{
	class DeferredRenderPipeline : public RenderPipelineBase
	{
	public:
		DeferredRenderPipeline(const RenderDevice_ptr& device, const RenderDescriptors_ptr& descriptors) :
			RenderPipelineBase(device, descriptors) {
		}
		~DeferredRenderPipeline();

		// ShaderObject
		struct ShaderObject
		{
			ShaderBase shaderBase;

			bool useNormalMap;
			Image normalMap;

			struct {
				uint32_t StandardSSBOIndex;
				uint32_t TextureId;
			}pushConstant;

			struct {
				StandardSSBO standardSSBO;
			}SSBO;
		};

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void CreateShaderObject(ShaderObject& shaderObject);
		void UpdateDescriptors(ShaderObject& shaderObject);

	private:
		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
	};
	TYPEDEF_SHARED_PTR_ARGS(DeferredRenderPipeline);
}