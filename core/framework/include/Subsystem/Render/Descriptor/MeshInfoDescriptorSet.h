// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>
#include "ClassPointer.h"
#include "DescriptorSetBase.h"
#include "MeshInfo.h"
#include "MeshletInfo.h"
#include "Mesh.h"
#include "Meshlet.h"
#include "PrimitiveGeometryShape.h"

namespace MyosotisFW::System::Render
{
	class MeshInfoDescriptorSet : public DescriptorSetBase
	{
	public:
		MeshInfoDescriptorSet(const RenderDevice_ptr& device, const VkDescriptorPool& descriptorPool);
		~MeshInfoDescriptorSet() {}

		enum class DescriptorBindingIndex : uint32_t
		{
			MeshInfo = 0,
			MeshletInfo,
			VertexData,
			UniqueIndexData,
			PrimitivesData,
			Count
		};

		void Update() override;
		uint32_t AddPrimitiveGeometry(const Shape::PrimitiveGeometryShape shape, const Mesh& meshData);
		MeshInfo GetMeshInfo(const uint32_t index) const { return m_meshInfo[index]; }

	private:
		void updateMeshInfo();
		void updateMeshletInfo();

		void updateVertexData();
		void updateUniqueIndexData();
		void updatePrimitivesData();

		std::vector<MeshInfo> m_meshInfo;
		std::vector<MeshletInfo> m_meshletInfo;

		std::vector<float> m_vertexData;
		std::vector<uint32_t> m_uniqueIndexData;
		std::vector<uint32_t> m_primitivesData;

		std::unordered_map<Shape::PrimitiveGeometryShape, uint32_t> m_primitiveMeshIDTable;
		std::unordered_map<std::string, uint32_t> m_customMeshIDTable;
	};

	TYPEDEF_SHARED_PTR_ARGS(MeshInfoDescriptorSet);
}