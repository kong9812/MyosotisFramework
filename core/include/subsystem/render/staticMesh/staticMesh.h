// Copyright (c) 2025 kong9812
#pragma once
#include <array>
#include <vulkan/vulkan.h>
#include "iglm.h"

#include "ObjectCast.h"
#include "ObjectBase.h"
#include "ClassPointer.h"
#include "Structs.h"
#include "RenderDevice.h"
#include "RenderResources.h"
#include "Camera.h"

namespace MyosotisFW::System::Render
{
	// todo.ファクトリーパターンにする予定
	// StaticMeshFactory
	// ・PrimitiveGeometryMesh
	// ・FBXMesh
	// ・...
	class StaticMesh : public ObjectBase
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

		virtual const ObjectType GetObjectType() const override { return ObjectType::Undefined; }

		virtual void PrepareForRender(RenderDevice_ptr device, RenderResources_ptr resources, StaticMeshShaderObject shaderObject);
		virtual void Update(const UpdateData& updateData, const Camera::CameraBase_ptr camera);
		virtual void BindCommandBuffer(VkCommandBuffer commandBuffer, bool transparent = false);

		virtual rapidjson::Value Serialize(rapidjson::Document::AllocatorType& allocator) const override;
		virtual void Deserialize(const rapidjson::Value& doc, std::function<void(ObjectType, const rapidjson::Value&)> createObject) override { __super::Deserialize(doc, createObject); }
		virtual glm::vec4 GetCullerData() { return glm::vec4(0.0f); }
	protected:
		virtual void loadAssets() {};
		virtual void prepareShaderStorageBuffers() {};

		// render device
		RenderDevice_ptr m_device;

		// render resources
		RenderResources_ptr m_resources;

		// vertex buffer
		std::array<std::vector<Buffer>, LOD::Max > m_vertexBuffer;
		// index buffer
		std::array<std::vector<Buffer>, LOD::Max> m_indexBuffer;

		// lod
		LOD m_currentLOD;
		std::array<float, LOD::Max> m_lodDistances;

		// shader object
		StaticMeshShaderObject m_staticMeshShaderObject;
	};
	TYPEDEF_SHARED_PTR(StaticMesh)
	OBJECT_CAST_FUNCTION(StaticMesh)
}
