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
#include "RenderPieplineList.h"

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

		StaticMeshShaderObject& GetStaticMeshShaderObject() { return m_staticMeshShaderObject; }

		virtual void PrepareForRender(const RenderDevice_ptr& device, const RenderResources_ptr& resources);
		virtual void Update(const UpdateData& updateData, const Camera::CameraBase_ptr& camera);
		virtual void BindCommandBuffer(const VkCommandBuffer& commandBuffer, const RenderPipelineType& pipelineType);

		virtual rapidjson::Value Serialize(rapidjson::Document::AllocatorType& allocator) const override { return __super::Serialize(allocator); }
		virtual void Deserialize(const rapidjson::Value& doc, const std::function<void(ObjectType, const rapidjson::Value&)>& createObject) override { __super::Deserialize(doc, createObject); }
		virtual glm::vec4 GetCullerData() { return glm::vec4(0.0f); }

		glm::vec3 GetLocalAABBMin() { return m_aabbMin; }
		glm::vec3 GetLocalAABBMax() { return m_aabbMax; }
		glm::vec3 GetWorldAABBMin() { return (m_aabbMin + m_transfrom.pos) * m_transfrom.scale; }
		glm::vec3 GetWorldAABBMax() { return (m_aabbMax + m_transfrom.pos) * m_transfrom.scale; }
		OBBData GetWorldOBBData();

	protected:
		virtual void loadAssets() {};
		virtual void prepareShaderStorageBuffers() {};

		// render device
		RenderDevice_ptr m_device;

		// render resources
		RenderResources_ptr m_resources;

		// vertex buffer
		std::array<std::vector<Buffer>, LOD::Max> m_vertexBuffer;
		// index buffer
		std::array<std::vector<Buffer>, LOD::Max> m_indexBuffer;

		// AABB
		glm::vec3 m_aabbMin;
		glm::vec3 m_aabbMax;

		// lod
		LOD m_currentLOD;
		std::array<float, LOD::Max> m_lodDistances;

		// shader object
		StaticMeshShaderObject m_staticMeshShaderObject;
	};
	TYPEDEF_SHARED_PTR(StaticMesh)
		OBJECT_CAST_FUNCTION(StaticMesh)
}
