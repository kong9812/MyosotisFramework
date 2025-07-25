// Copyright (c) 2025 kong9812
#pragma once
#include <array>
#include <vulkan/vulkan.h>
#include "iglm.h"

#include "ComponentCast.h"
#include "ComponentBase.h"
#include "ClassPointer.h"
#include "Structs.h"
#include "RenderPieplineList.h"
#include "DeferredRenderPipeline.h"
#include "ShadowMapRenderPipeline.h"

namespace MyosotisFW::System::Render
{
	// 前方宣言
	class RenderDevice;
	TYPEDEF_SHARED_PTR_FWD(RenderDevice);
	class RenderResources;
	TYPEDEF_SHARED_PTR_FWD(RenderResources);
	namespace Camera
	{
		class CameraBase;
		TYPEDEF_SHARED_PTR_FWD(CameraBase);
	}

	// todo.ファクトリーパターンにする予定
	// StaticMeshFactory
	// ・PrimitiveGeometryMesh
	// ・FBXMesh
	// ・...
	class StaticMesh : public ComponentBase
	{
	public:
		//  todo.初期化でrenderpipelineとdescriptorをとってくるのがいいかも
		StaticMesh();
		~StaticMesh();

		typedef enum {
			Hide = -1,
			LOD1,
			LOD2,
			LOD3,
			Max
		} LOD;

		const ComponentType GetType() const override { return ComponentType::Undefined; }

		DeferredRenderPipeline::ShaderObject& GetStaticMeshShaderObject() { return m_staticMeshShaderObject; }
		ShadowMapRenderPipeline::ShaderObject& GetShadowMapShaderObject() { return m_shadowMapShaderObject; }

		virtual void PrepareForRender(const RenderDevice_ptr& device, const RenderResources_ptr& resources);
		virtual void Update(const UpdateData& updateData, const Camera::CameraBase_ptr& camera);
		virtual void BindCommandBuffer(const VkCommandBuffer& commandBuffer, const RenderPipelineType& pipelineType);

	protected:
		virtual void loadAssets() {}
		virtual void prepareShaderStorageBuffers() {}

		// render device
		RenderDevice_ptr m_device;

		// render resources
		RenderResources_ptr m_resources;

		// vertex buffer
		std::array<std::vector<Buffer>, LOD::Max> m_vertexBuffer;
		// index buffer
		std::array<std::vector<Buffer>, LOD::Max> m_indexBuffer;

		// lod
		LOD m_currentLOD;
		std::array<float, LOD::Max> m_lodDistances;

		// shader object
		ShadowMapRenderPipeline::ShaderObject m_shadowMapShaderObject;
		DeferredRenderPipeline::ShaderObject m_staticMeshShaderObject;
	};
	TYPEDEF_SHARED_PTR(StaticMesh);
	OBJECT_CAST_FUNCTION(StaticMesh);
}
