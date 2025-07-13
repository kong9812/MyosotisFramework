// Copyright (c) 2025 kong9812
#pragma once
#include <array>
#include <vector>
#include "Structs.h"
#include "ClassPointer.h"
#include "RenderDevice.h"
#include "RenderResources.h"

namespace MyosotisFW::System::Render {
	// todo.後で全部消す…構造を変える
	// ・[検討中] Object -> RenderComponentBase、そしてStageObjectを新しく作る？…
	//　・これが一番早い気がする…
	//	　・StageObject
	//	　　・SerializeとDeserialize
	//	　　・複数Componentの合算結果を返す
	//	　　・AABB / OBB
	//	　　・TextureList ? DiscriptorList ?
	//	　　・その他…

	typedef struct
	{
		glm::vec3 pos;
		glm::vec3 rot;
		glm::vec3 scale;
	}Transform;

	class RenderComponentBase
	{
	public:
		RenderComponentBase() {}
		~RenderComponentBase() {}

		typedef enum {
			Hide = -1,
			LOD1,
			LOD2,
			LOD3,
			Max
		} LOD;

		glm::vec3 GetLocalAABBMin() { return m_aabbMin; }
		glm::vec3 GetLocalAABBMax() { return m_aabbMax; }
		glm::vec3 GetWorldAABBMin() { return (m_aabbMin + m_transfrom.pos) * m_transfrom.scale; }
		glm::vec3 GetWorldAABBMax() { return (m_aabbMax + m_transfrom.pos) * m_transfrom.scale; }
		OBBData GetWorldOBBData();

	protected:
		Transform m_transfrom;

	protected:
		virtual void loadAssets() {}
		virtual void prepareShaderStorageBuffers() {}

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
	TYPEDEF_SHARED_PTR(RenderComponentBase)
}