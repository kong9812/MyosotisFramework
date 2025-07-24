// Copyright (c) 2025 kong9812
#pragma once
#include "ComponentBase.h"
#include "ComponentCast.h"
#include "RenderPieplineList.h"
#include "SkyboxRenderPipeline.h"

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

	class Skybox : public ComponentBase
	{
	public:
		Skybox();
		~Skybox();

		virtual const ComponentType GetType() const override { return ComponentType::Skybox; }

		void PrepareForRender(const RenderDevice_ptr& device, const RenderResources_ptr& resources);
		void Update(const UpdateData& updateData, const Camera::CameraBase_ptr& camera);
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer);
		SkyboxRenderPipeline::SkyboxShaderObject& GetSkyboxShaderObject() { return m_skyboxShaderObject; }

	private:
		void loadAssets();

		// render device
		RenderDevice_ptr m_device;

		// render resources
		RenderResources_ptr m_resources;

		// vertex buffer
		Buffer m_vertexBuffer;
		// index buffer
		Buffer m_indexBuffer;

		// shader object
		SkyboxRenderPipeline::SkyboxShaderObject m_skyboxShaderObject;
	};
	TYPEDEF_SHARED_PTR(Skybox);
	OBJECT_CAST_FUNCTION(Skybox);
}