// Copyright (c) 2025 kong9812
#pragma once
#include "StaticMesh.h"
#include "ObjectCast.h"

namespace MyosotisFW::System::Render
{
	class InteriorObject : public StaticMesh
	{
	public:
		InteriorObject();
		~InteriorObject() {};

		virtual const ObjectType GetObjectType() const override { return ObjectType::InteriorObjectMesh; }

		void PrepareForRender(const RenderDevice_ptr& device, const RenderResources_ptr& resources) override;
		void Update(const UpdateData& updateData, const Camera::CameraBase_ptr& camera) override;
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer, const RenderPipelineType& pipelineType) override;

		glm::vec4 GetCullerData() override;
		virtual rapidjson::Value Serialize(rapidjson::Document::AllocatorType& allocator) const override { return __super::Serialize(allocator); }
		virtual void Deserialize(const rapidjson::Value& doc, const std::function<void(ObjectType, const rapidjson::Value&)>& createObject) override { __super::Deserialize(doc, createObject); }
	private:
		void loadAssets() override;
		void prepareShaderStorageBuffers() override {};
	};
	TYPEDEF_SHARED_PTR(InteriorObject)
		OBJECT_CAST_FUNCTION(InteriorObject)
}