// Copyright (c) 2025 kong9812
#pragma once
#include <vector>
#include "DescriptorPool.h"
#include "SceneInfoDescriptorSet.h"
#include "ObjectInfoDescriptorSet.h"
#include "MeshInfoDescriptorSet.h"
#include "TextureDescriptorSet.h"
#include "RayTracingDescriptorSet.h"
#include "MaterialDescriptorSet.h"

namespace MyosotisFW::System::Render
{
	class RenderDevice;
	TYPEDEF_SHARED_PTR_FWD(RenderDevice);

	class RenderDescriptors
	{
	public:
		RenderDescriptors(const RenderDevice_ptr& device);
		~RenderDescriptors() {}

		const SceneInfoDescriptorSet_ptr GetSceneInfoDescriptorSet() const { return m_sceneInfoDescriptorSet; }
		const ObjectInfoDescriptorSet_ptr GetObjectInfoDescriptorSet() const { return m_objectInfoDescriptorSet; }
		const MeshInfoDescriptorSet_ptr GetMeshInfoDescriptorSet() const { return m_meshInfoDescriptorSet; }
		const TextureDescriptorSet_ptr GetTextureDescriptorSet() const { return m_textureDescriptorSet; }
		const RayTracingDescriptorSet_ptr GetRayTracingDescriptorSet() const { return m_rayTracingDescriptorSet; }
		const MaterialDescriptorSet_ptr GetMaterialDescriptorSet() const { return m_materialDescriptorSet; }

		std::vector<VkDescriptorSet> GetDescriptorSet();
		std::vector<VkDescriptorSetLayout> GetDescriptorSetLayout();

	private:
		DescriptorPool_ptr m_descriptorPool;

		SceneInfoDescriptorSet_ptr m_sceneInfoDescriptorSet;
		ObjectInfoDescriptorSet_ptr m_objectInfoDescriptorSet;
		MeshInfoDescriptorSet_ptr m_meshInfoDescriptorSet;
		TextureDescriptorSet_ptr m_textureDescriptorSet;
		RayTracingDescriptorSet_ptr m_rayTracingDescriptorSet;
		MaterialDescriptorSet_ptr m_materialDescriptorSet;
	};
	TYPEDEF_SHARED_PTR_ARGS(RenderDescriptors);
}