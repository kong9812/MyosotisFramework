// Copyright (c) 2025 kong9812
#pragma once
#include <vector>
#include "RenderPipelineBase.h"
#include "AxisDrawCommand.h"

namespace MyosotisFW::System::Render
{
	// 前方宣言
	namespace Camera
	{
		class CameraBase;
		TYPEDEF_SHARED_PTR_FWD(CameraBase);
	}

	class GizmoPipeline : public RenderPipelineBase
	{
	public:
		GizmoPipeline(const RenderDevice_ptr& device, const RenderDescriptors_ptr& renderDescriptors) :
			RenderPipelineBase(device, renderDescriptors),
			pushConstant({}),
			m_baseModelMatrix(glm::mat4(0.0f)),
			m_vertexBuffer({}),
			m_indexBuffer({}) {
		}
		~GizmoPipeline();

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer, const std::vector<AxisDrawCommand>& axisDrawCommand);

	private:
		struct PushConstant {
			glm::mat4 model = glm::mat4(0.0f);
			glm::vec4 color = glm::vec4(0.0f);
		}pushConstant;

		glm::mat4 m_baseModelMatrix;
		Buffer m_vertexBuffer;
		Buffer m_indexBuffer;

		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
	};
	TYPEDEF_SHARED_PTR_ARGS(GizmoPipeline);
}