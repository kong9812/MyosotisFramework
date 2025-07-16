// Copyright (c) 2025 kong9812
#pragma once
#include "StaticMesh.h"
#include "ComponentCast.h"
#include "PrimitiveGeometryShape.h"

namespace MyosotisFW::System::Render
{
	class PrimitiveGeometry : public StaticMesh
	{
	public:
		PrimitiveGeometry();
		~PrimitiveGeometry() {}

		virtual const ComponentType GetType() const override { return ComponentType::PrimitiveGeometryMesh; }

		void PrepareForRender(const RenderDevice_ptr& device, const RenderResources_ptr& resources) override;
		void Update(const UpdateData& updateData, const Camera::CameraBase_ptr& camera) override;
		void BindCommandBuffer(const VkCommandBuffer& commandBuffer, const RenderPipelineType& pipelineType) override;

		glm::vec4 GetCullerData() override;
		virtual rapidjson::Value Serialize(rapidjson::Document::AllocatorType& allocator) const override;
		virtual void Deserialize(const rapidjson::Value& doc) override;
	private:
		void loadAssets() override;
		void prepareShaderStorageBuffers() override {};

		Shape::PrimitiveGeometryShape m_primitiveGeometryShape;
	};
	TYPEDEF_SHARED_PTR(PrimitiveGeometry);
	OBJECT_CAST_FUNCTION(PrimitiveGeometry);
}