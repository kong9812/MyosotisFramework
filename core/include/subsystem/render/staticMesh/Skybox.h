// Copyright (c) 2025 kong9812
#pragma once
#include "ObjectBase.h"
#include "ObjectCast.h"
#include "RenderDevice.h"
#include "RenderResources.h"
#include "Camera.h"
#include "RenderPieplineList.h"

namespace MyosotisFW::System::Render
{
	class Skybox : public ObjectBase
	{
	public:
		Skybox();
		~Skybox();

		virtual const ObjectType GetObjectType() const override { return ObjectType::Skybox; }

		void PrepareForRender(RenderDevice_ptr device, RenderResources_ptr resources);
		void Update(const UpdateData& updateData, const Camera::CameraBase_ptr camera);
		void BindCommandBuffer(VkCommandBuffer commandBuffer);
		SkyboxShaderObject& GetSkyboxShaderObject() { return m_skyboxShaderObject; }
		virtual rapidjson::Value Serialize(rapidjson::Document::AllocatorType& allocator) const;
		virtual void Deserialize(const rapidjson::Value& doc, std::function<void(ObjectType, const rapidjson::Value&)> createObject);
	private:
		void loadAssets();
		void prepareShaderStorageBuffers() {};

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