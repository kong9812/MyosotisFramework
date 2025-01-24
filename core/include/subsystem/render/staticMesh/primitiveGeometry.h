// Copyright (c) 2025 kong9812
#pragma once
#include "staticMesh.h"
#include "objectCast.h"

namespace MyosotisFW::System::Render
{
	class PrimitiveGeometry : public StaticMesh
	{
	public:
		PrimitiveGeometry();
		~PrimitiveGeometry() {};

		void PrepareForRender(RenderDevice_ptr device, RenderResources_ptr resources, VkRenderPass renderPass, VkPipelineCache pipelineCache);
		void Update(const Utility::Vulkan::Struct::UpdateData& updateData, const Camera::CameraBase_ptr camera) override;
		void BindCommandBuffer(VkCommandBuffer commandBuffer) override;
		void BindDebugGUIElement() override {};

		virtual rapidjson::Value Serialize(rapidjson::Document::AllocatorType& allocator) const { return __super::Serialize(allocator); }
		virtual void Deserialize(const rapidjson::Value& doc, std::function<void(ObjectType, const rapidjson::Value&)> createObject) { __super::Deserialize(doc, createObject); }
	private:
		void loadAssets() override;
		void prepareUniformBuffers() override;
		void prepareShaderStorageBuffers() override {};

		void prepareDescriptors() override;
		void prepareRenderPipeline() override;

	};
	TYPEDEF_SHARED_PTR(PrimitiveGeometry)
	OBJECT_CAST_FUNCTION(PrimitiveGeometry)
}