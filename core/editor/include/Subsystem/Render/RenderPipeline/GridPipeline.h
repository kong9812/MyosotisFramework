// Copyright (c) 2025 kong9812
#pragma once
#include <vector>
#include "RenderPipelineBase.h"

namespace MyosotisFW::System::Render
{
	// 前方宣言
	namespace Camera
	{
		class CameraBase;
		TYPEDEF_SHARED_PTR_FWD(CameraBase);
	}

	class GridPipeline : public RenderPipelineBase
	{
	public:
		GridPipeline(const RenderDevice_ptr& device, const RenderDescriptors_ptr& renderDescriptors) :
			RenderPipelineBase(device, renderDescriptors),
			pushConstant({}) {
		}
		~GridPipeline();

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer);
		void Update(const Camera::CameraBase_ptr& camera);

	private:
		struct PushConstant {
			glm::mat4 invVP = glm::mat4(0.0f);
		}pushConstant;

		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
	};
	TYPEDEF_SHARED_PTR_ARGS(GridPipeline);
}