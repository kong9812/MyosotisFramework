// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
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
		void AddPrimitiveGeometry(const std::vector<std::pair<Shape::PrimitiveGeometryShape, std::vector<Mesh>>>& meshDatas);

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
	};

	TYPEDEF_SHARED_PTR_ARGS(MeshInfoDescriptorSet);
}