// Copyright (c) 2025 kong9812
#pragma once
#include "StaticMesh.h"
#include "ComponentCast.h"

namespace MyosotisFW::System::Render
{
	class InteriorObject : public ComponentBase
	{
	public:
		InteriorObject();
		~InteriorObject();

		virtual const ComponentType GetType() const override { return ComponentType::InteriorObjectMesh; }

		void PrepareForRender(const RenderDevice_ptr& device, const RenderResources_ptr& resources);
		void Update(const UpdateData& updateData, const Camera::CameraBase_ptr& camera);
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer);
		InteriorObjectShaderObject& GetInteriorObjectShaderObject() { return m_interiorObjectShaderObject; }
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
		InteriorObjectShaderObject m_interiorObjectShaderObject;
	};
	TYPEDEF_SHARED_PTR(InteriorObject)
		OBJECT_CAST_FUNCTION(InteriorObject)
}