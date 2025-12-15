// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include <fstream>
#include <vector>
#include <chrono>
#include <unordered_set>

#include "iofbx.h"
#include "AppInfo.h"

#include "Logger.h"
#include "Mesh.h"
#include "imeshoptimizer.h"
#include "ixatlas.h"

namespace Utility::Loader {
	inline ofbx::u32 triangulate(const ofbx::GeometryData& geom, const ofbx::GeometryPartition::Polygon& polygon, std::vector<uint32_t>& tri_indices) {
		// 必要な三角形数を計算
		ofbx::u32 triangleCount = polygon.vertex_count - 2;
		if (triangleCount <= 0) return 0;

		// tri_indices のサイズを確保
		tri_indices.reserve(tri_indices.size() + 3 * triangleCount);

		if (polygon.vertex_count == 3) {
			tri_indices.push_back(static_cast<uint32_t>(polygon.from_vertex));
			tri_indices.push_back(static_cast<uint32_t>(polygon.from_vertex + 1));
			tri_indices.push_back(static_cast<uint32_t>(polygon.from_vertex + 2));
		}
		else if (polygon.vertex_count == 4) {
			// 四角形を三角形に分割
			tri_indices.push_back(static_cast<uint32_t>(polygon.from_vertex + 0));
			tri_indices.push_back(static_cast<uint32_t>(polygon.from_vertex + 1));
			tri_indices.push_back(static_cast<uint32_t>(polygon.from_vertex + 2));

			tri_indices.push_back(static_cast<uint32_t>(polygon.from_vertex + 0));
			tri_indices.push_back(static_cast<uint32_t>(polygon.from_vertex + 2));
			tri_indices.push_back(static_cast<uint32_t>(polygon.from_vertex + 3));
		}
		else {
			// 多角形をトライアングルファンで分割
			for (ofbx::u32 tri = 0; tri < triangleCount; ++tri) {
				tri_indices.push_back(static_cast<uint32_t>(polygon.from_vertex));
				tri_indices.push_back(static_cast<uint32_t>(polygon.from_vertex) + 1 + tri);
				tri_indices.push_back(static_cast<uint32_t>(polygon.from_vertex) + 2 + tri);
			}
		}
		return 3 * triangleCount;
	}

	inline std::vector<MyosotisFW::Mesh> loadFbx(std::string fileName)
	{
#ifdef DEBUG
		Logger::Debug("[VK_Loader] Start load: " + fileName);
		auto start = std::chrono::high_resolution_clock::now();
#endif

		std::ifstream file(MyosotisFW::AppInfo::g_modelFolder + fileName, std::ios::ate | std::ios::binary);
		ASSERT(file.is_open(), "Failed to open fbx file: " + std::string(MyosotisFW::AppInfo::g_modelFolder) + fileName);
		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buf(fileSize);
		file.seekg(0);
		file.read(buf.data(), fileSize);
		file.close();

		ofbx::IScene* scene = ofbx::load(
			reinterpret_cast<ofbx::u8*>(buf.data()),
			static_cast<ofbx::usize>(fileSize),
			static_cast<ofbx::u16>(ofbx::LoadFlags::NONE));

		std::vector<MyosotisFW::Mesh> meshes{};

		uint32_t meshCount = scene->getMeshCount();
		std::vector<int> testList{};
		for (uint32_t meshIdx = 0; meshIdx < meshCount; meshIdx++)
		{
			MyosotisFW::Mesh meshData{};
			bool firstDataForAABB = true;
			const ofbx::Mesh* mesh = scene->getMesh(meshIdx);
			const ofbx::GeometryData& geomData = mesh->getGeometryData();
			const ofbx::Vec3Attributes positions = geomData.getPositions();
			const ofbx::Vec3Attributes normal = geomData.getNormals();
			const ofbx::Vec2Attributes uv = geomData.getUVs();
			const ofbx::Vec4Attributes color = geomData.getColors();

			for (uint32_t partitionIdx = 0; partitionIdx < geomData.getPartitionCount(); partitionIdx++)
			{
				const ofbx::GeometryPartition& geometryPartition = geomData.getPartition(partitionIdx);
				for (uint32_t polygonIdx = 0; polygonIdx < geometryPartition.polygon_count; polygonIdx++)
				{
					const ofbx::GeometryPartition::Polygon& polygon = geometryPartition.polygons[polygonIdx];

					for (uint32_t vertexIdx = polygon.from_vertex; vertexIdx < polygon.from_vertex + polygon.vertex_count; vertexIdx++)
					{
						glm::vec3 v = ToGlmVec3(positions.get(vertexIdx));
						glm::vec3 n = glm::vec3(0.0f);
						glm::vec2 u0 = glm::vec2(0.0f);
						glm::vec2 u1 = glm::vec2(0.0f);
						glm::vec4 c = glm::vec4(1.0f);

						// Normal
						if (normal.count > vertexIdx)
						{
							n = ToGlmVec3(normal.get(vertexIdx));
						}

						// UV0
						if (uv.count > vertexIdx)
						{
							u0 = ToGlmVec2(uv.get(vertexIdx));
						}

						// Color
						if (color.count > vertexIdx)
						{
							c = ToGlmVec4(color.get(vertexIdx));
						}

						// aabb
						if (firstDataForAABB)
						{
							meshData.meshInfo.AABBMin.x = v.x;
							meshData.meshInfo.AABBMin.y = v.y;
							meshData.meshInfo.AABBMin.z = v.z;
							meshData.meshInfo.AABBMax.x = v.x;
							meshData.meshInfo.AABBMax.y = v.y;
							meshData.meshInfo.AABBMax.z = v.z;
							firstDataForAABB = false;
						}
						else
						{
							meshData.meshInfo.AABBMin.x = meshData.meshInfo.AABBMin.x < v.x ? meshData.meshInfo.AABBMin.x : v.x;
							meshData.meshInfo.AABBMin.y = meshData.meshInfo.AABBMin.y < v.y ? meshData.meshInfo.AABBMin.y : v.y;
							meshData.meshInfo.AABBMin.z = meshData.meshInfo.AABBMin.z < v.z ? meshData.meshInfo.AABBMin.z : v.z;
							meshData.meshInfo.AABBMax.x = meshData.meshInfo.AABBMax.x > v.x ? meshData.meshInfo.AABBMax.x : v.x;
							meshData.meshInfo.AABBMax.y = meshData.meshInfo.AABBMax.y > v.y ? meshData.meshInfo.AABBMax.y : v.y;
							meshData.meshInfo.AABBMax.z = meshData.meshInfo.AABBMax.z > v.z ? meshData.meshInfo.AABBMax.z : v.z;
						}

						meshData.vertex.insert(meshData.vertex.end(), { v, n, u0, u1, c });
					}
				}

				// 本来のIndex
				std::vector<uint32_t> index{};
				for (uint32_t polygonIdx = 0; polygonIdx < geometryPartition.polygon_count; polygonIdx++)
				{
					const ofbx::GeometryPartition::Polygon& polygon = geometryPartition.polygons[polygonIdx];
					int triangle[3]{};
					triangulate(geomData, polygon, triangle);
					for (uint32_t i = 0; i < 3; i++)
					{
						index.push_back(static_cast<uint32_t>(triangle[i]));
					}
				}

				// UV1
				meshData.meshInfo.atlasSize = xatlas::BuildLightmapUV(meshData.vertex, index);

				// Meshlet
				meshoptimizer::BuildMeshletData(meshData, index,
					MyosotisFW::AppInfo::g_maxMeshletVertices,
					MyosotisFW::AppInfo::g_maxMeshletPrimitives);

				// AABB
				meshData.meshInfo.AABBMin = glm::vec4(FLT_MAX);
				meshData.meshInfo.AABBMax = glm::vec4(-FLT_MAX);
				for (const MyosotisFW::Meshlet& meshlet : meshData.meshlet)
				{
					meshData.meshInfo.AABBMin = glm::min(meshData.meshInfo.AABBMin, meshlet.meshletInfo.AABBMin);
					meshData.meshInfo.AABBMax = glm::max(meshData.meshInfo.AABBMax, meshlet.meshletInfo.AABBMax);
				}

				// MeshInfo更新
				meshData.meshInfo.meshletCount = static_cast<uint32_t>(meshData.meshlet.size());
				meshData.meshInfo.vertexFloatCount = static_cast<uint32_t>(meshData.vertex.size()) * (sizeof(MyosotisFW::VertexData) / sizeof(float));

				meshes.push_back(meshData);
			}
			scene->destroy();
#ifdef DEBUG
			Logger::Debug("[VK_Loader] End load: " + fileName +
				"(" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count()) + "ms)");
#endif
		}
		return meshes;
	}
}