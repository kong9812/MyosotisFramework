// Copyright (c) 2025 kong9812
#pragma once
#include "RenderPipelineBase.h"
#include "Structs.h"

namespace MyosotisFW::System::Render
{
	class MeshShaderRenderPipeline : public RenderPipelineBase
	{
	public:
		MeshShaderRenderPipeline(const RenderDevice_ptr& device, const RenderDescriptors_ptr& descriptors) :
			RenderPipelineBase(device, descriptors) {
		}
		~MeshShaderRenderPipeline();

		// ShaderObject
		struct ShaderObject
		{
			ShaderBase shaderBase;
		};

		void Initialize(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer);

	private:
		struct VertexDataMetaData {
			uint32_t vertexCount;
			uint32_t primitiveCount;    // 三角形単位(三角形の数)
			uint32_t vertexAttributeBit;
			uint32_t unitSize;          // 一枚当たりのサイズ
			uint32_t offset;
		};

		struct IndexDataMetaData {
			uint32_t offset;            // IndexDataの開始位置
		};

		struct VertexData {
			glm::vec4 position;
			glm::vec3 normal;
			glm::vec2 uv;
			glm::vec4 color;
			glm::vec4 tangent;
		};

	private:
		void prepareRenderPipeline(const RenderResources_ptr& resources, const VkRenderPass& renderPass) override;
		void prepareDescriptorSet();

		VkDescriptorSetLayout m_descriptorSetLayout;
		VkDescriptorSet m_descriptorSet;

		Mesh m_vertex;

		Buffer m_vertexMetaDataBuffer;
		Buffer m_vertexDataBuffer;
		Buffer m_indexMetaDataBuffer;
		Buffer m_indexDataBuffer;

		PFN_vkCmdDrawMeshTasksEXT m_vkCmdDrawMeshTasksEXT;
	};
	TYPEDEF_SHARED_PTR_ARGS(MeshShaderRenderPipeline);
}