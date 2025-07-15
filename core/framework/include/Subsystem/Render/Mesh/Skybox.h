// Copyright (c) 2025 kong9812
#pragma once
#include "ComponentBase.h"
#include "ComponentCast.h"
#include "RenderDevice.h"
#include "RenderResources.h"
#include "Camera.h"
#include "RenderPieplineList.h"

namespace MyosotisFW::System::Render
{
	class Skybox : public ComponentBase
	{
	public:
		Skybox();
		~Skybox();

		virtual const ComponentType GetType() const override { return ComponentType::Skybox; }

		void PrepareForRender(const RenderDevice_ptr& device, const RenderResources_ptr& resources);
		void Update(const UpdateData& updateData, const Camera::CameraBase_ptr& camera);
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer);
		SkyboxShaderObject& GetSkyboxShaderObject() { return m_skyboxShaderObject; }

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
		SkyboxShaderObject m_skyboxShaderObject;
	};
	TYPEDEF_SHARED_PTR(Skybox)
		OBJECT_CAST_FUNCTION(Skybox)
}