// Copyright (c) 2025 kong9812
#pragma once
#include "StaticMesh.h"
#include "ObjectCast.h"

namespace MyosotisFW::System::Render
{
	class InteriorObject : public ObjectBase
	{
	public:
		InteriorObject();
		~InteriorObject();

		virtual const ObjectType GetObjectType() const override { return ObjectType::InteriorObjectMesh; }

		void PrepareForRender(const RenderDevice_ptr& device, const RenderResources_ptr& resources);
		void Update(const UpdateData& updateData, const Camera::CameraBase_ptr& camera);
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer);
		InteriorObjectShaderObject& GetInteriorObjectShaderObject() { return m_interiorObjectShaderObject; }
		rapidjson::Value Serialize(rapidjson::Document::AllocatorType& allocator) const override { return __super::Serialize(allocator); };
		void Deserialize(const rapidjson::Value& doc, const std::function<void(ObjectType, const rapidjson::Value&)>& createObject) override { __super::Deserialize(doc, createObject); };
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