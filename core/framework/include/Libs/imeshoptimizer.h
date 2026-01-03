// Copyright (c) 2025 kong9812
// For include meshoptimizer
#pragma once
#include <meshoptimizer.h>
#include <vector>
#include "Mesh.h"

namespace meshoptimizer
{
	inline void BuildMeshletData(MyosotisFW::Mesh& meshData, const std::vector<uint32_t>& index,
		const uint32_t maxMeshletVertices, const uint32_t maxMeshletPrimitives)
	{
		// メッシュレット数の上限
		size_t maxMeshlets = meshopt_buildMeshletsBound(
			index.size(),
			maxMeshletVertices,
			maxMeshletPrimitives);

		// meshoptimizer用データ作成
		std::vector<meshopt_Meshlet> meshlets(maxMeshlets);
		std::vector<uint32_t> meshletVertices(maxMeshlets * maxMeshletVertices);
		std::vector<uint8_t> meshletTriangles(maxMeshlets * maxMeshletPrimitives * 3);

		// メッシュレット生成
		size_t meshletCount = meshopt_buildMeshlets(
			meshlets.data(),
			meshletVertices.data(),
			meshletTriangles.data(),
			index.data(),
			index.size(),
			&meshData.vertex[0].position.x,
			meshData.vertex.size(),
			sizeof(MyosotisFW::VertexData),
			maxMeshletVertices,
			maxMeshletPrimitives,
			0.0f);

		meshData.meshlet.reserve(meshletCount);
		for (size_t i = 0; i < meshletCount; i++)
		{
			const meshopt_Meshlet& src = meshlets[i];
			MyosotisFW::Meshlet dst{};

			// UniqueIndex (GlobalIndex)
			dst.uniqueIndex.reserve(src.vertex_count);
			for (size_t v = 0; v < src.vertex_count; v++)
			{
				uint32_t vertexIndex = meshletVertices[src.vertex_offset + v];
				dst.uniqueIndex.push_back(vertexIndex);
			}

			// Primitives (LocalIndex)
			dst.primitives.reserve(static_cast<size_t>(src.triangle_count) * 3);
			for (size_t t = 0; t < static_cast<size_t>(src.triangle_count) * 3; t++)
			{
				uint32_t triangleIndex = static_cast<uint32_t>(meshletTriangles[src.triangle_offset + t]);
				dst.primitives.push_back(triangleIndex);
			}

			// AABB
			glm::vec3 p0 = meshData.vertex[meshletVertices[src.vertex_offset + 0]].position;
			dst.meshletInfo.AABBMin = glm::vec4(p0, 0.0f);
			dst.meshletInfo.AABBMax = glm::vec4(p0, 0.0f);
			for (size_t v = 1; v < src.vertex_count; v++)
			{
				uint32_t vertexIndex = meshletVertices[src.vertex_offset + v];
				const glm::vec3& pos = meshData.vertex[vertexIndex].position;
				dst.meshletInfo.AABBMin = glm::min(dst.meshletInfo.AABBMin, glm::vec4(pos, 0.0f));
				dst.meshletInfo.AABBMax = glm::max(dst.meshletInfo.AABBMax, glm::vec4(pos, 0.0f));
			}

			dst.meshletInfo.vertexCount = src.vertex_count;
			dst.meshletInfo.primitiveCount = src.triangle_count;
			meshData.meshInfo.meshletCount = meshletCount;
			meshData.meshlet.push_back(dst);
		}
	}
}