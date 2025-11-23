// Copyright (c) 2025 kong9812
#pragma once
#include <array>
#include <vulkan/vulkan.h>
#include "iglm.h"

#include "ComponentCast.h"
#include "ComponentBase.h"
#include "ClassPointer.h"
#include "Structs.h"
#include "VBDispatchInfo.h"

namespace MyosotisFW::System::Render
{
	// 前方宣言
	class RenderDevice;
	TYPEDEF_SHARED_PTR_FWD(RenderDevice);
	class RenderResources;
	TYPEDEF_SHARED_PTR_FWD(RenderResources);
	class MeshInfoDescriptorSet;
	TYPEDEF_SHARED_PTR_FWD(MeshInfoDescriptorSet);

	namespace Camera
	{
		class CameraBase;
		TYPEDEF_SHARED_PTR_FWD(CameraBase);
	}

	// todo.ファクトリーパターンにする予定
	// StaticMeshFactory
	// ・PrimitiveGeometryMesh
	// ・FBXMesh
	// ・...
	class StaticMesh : public ComponentBase
	{
	public:
		//  todo.初期化でrenderpipelineとdescriptorをとってくるのがいいかも
		StaticMesh(const uint32_t objectID);
		~StaticMesh();

		typedef enum {
			Hide = -1,
			LOD1,
			LOD2,
			LOD3,
			Max
		} LOD;

		const ComponentType GetType() const override { return ComponentType::Undefined; }

		virtual void PrepareForRender(const RenderDevice_ptr& device, const RenderResources_ptr& resources, const MeshInfoDescriptorSet_ptr& meshInfoDescriptorSet);
		virtual void Update(const UpdateData& updateData, const Camera::CameraBase_ptr& camera);
		virtual void BindCommandBuffer(const VkCommandBuffer& commandBuffer);
		std::vector<VBDispatchInfo>& GetVBDispatchInfo() { return m_vbDispatchInfo; }

		glm::vec3 GetLocalAABBMin() { return m_aabbMin; }
		glm::vec3 GetLocalAABBMax() { return m_aabbMax; }

		virtual bool IsStaticMesh() const override { return true; }

	protected:
		virtual void loadAssets() {}
		virtual void prepareShaderStorageBuffers() {}

		// render device
		RenderDevice_ptr m_device;

		// render resources
		RenderResources_ptr m_resources;

		// 
		MeshInfoDescriptorSet_ptr m_meshInfoDescriptorSet;

		std::vector<VBDispatchInfo> m_vbDispatchInfo;

		// AABB
		glm::vec3 m_aabbMin;
		glm::vec3 m_aabbMax;
	};
	TYPEDEF_SHARED_PTR_ARGS(StaticMesh);
	OBJECT_CAST_FUNCTION(StaticMesh);
}
