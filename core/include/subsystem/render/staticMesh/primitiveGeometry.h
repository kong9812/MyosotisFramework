// Copyright (c) 2025 kong9812
#pragma once
#include "StaticMesh.h"
#include "ObjectCast.h"

namespace MyosotisFW::System::Render
{
	class PrimitiveGeometry : public StaticMesh
	{
	public:
		PrimitiveGeometry();
		~PrimitiveGeometry() {};

		virtual const ObjectType GetObjectType() const override { return ObjectType::PrimitiveGeometryMesh; }

		void PrepareForRender(RenderDevice_ptr device, RenderResources_ptr resources)override;
		void Update(const UpdateData& updateData, const Camera::CameraBase_ptr camera) override;
		void BindCommandBuffer(VkCommandBuffer commandBuffer, bool transparent = false) override;

		glm::vec4 GetCullerData() override;
		virtual rapidjson::Value Serialize(rapidjson::Document::AllocatorType& allocator) const { return __super::Serialize(allocator); }
		virtual void Deserialize(const rapidjson::Value& doc, std::function<void(ObjectType, const rapidjson::Value&)> createObject) { __super::Deserialize(doc, createObject); }
	private:
		void loadAssets() override;
		void prepareShaderStorageBuffers() override {};
	};
	TYPEDEF_SHARED_PTR(PrimitiveGeometry)
		OBJECT_CAST_FUNCTION(PrimitiveGeometry)
}