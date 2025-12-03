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
		std::vector<glm::vec3> tmpPositions{};

		uint32_t meshCount = scene->getMeshCount();
		std::vector<int> testList{};
		for (uint32_t meshIdx = 0; meshIdx < meshCount; meshIdx++)
		{
			MyosotisFW::Mesh meshData{};
			bool firstDataForAABB = true;
			const ofbx::Mesh* mesh = scene->getMesh(meshIdx);
			const ofbx::GeometryData& geomData = mesh->getGeometryData();
			const ofbx::Vec3Attributes positions = geomData.getPositions();
			for (uint32_t partitionIdx = 0; partitionIdx < geomData.getPartitionCount(); partitionIdx++)
			{
				const ofbx::GeometryPartition& geometryPartition = geomData.getPartition(partitionIdx);
				for (uint32_t polygonIdx = 0; polygonIdx < geometryPartition.polygon_count; polygonIdx++)
				{
					const ofbx::GeometryPartition::Polygon& polygon = geometryPartition.polygons[polygonIdx];

					for (uint32_t vertexIdx = polygon.from_vertex; vertexIdx < polygon.from_vertex + polygon.vertex_count; vertexIdx++)
					{
						ofbx::Vec3 v = positions.get(vertexIdx);
						meshData.vertex.push_back(v.x);
						meshData.vertex.push_back(v.y);
						meshData.vertex.push_back(v.z);
						meshData.vertex.push_back(1.0f);
						tmpPositions.push_back(glm::vec4(v.x, v.y, v.z, 0.0f));

						// 仮normal
						meshData.vertex.push_back(0.0f);
						meshData.vertex.push_back(0.0f);
						meshData.vertex.push_back(0.0f);

						// 仮uv
						meshData.vertex.push_back(0.0f);
						meshData.vertex.push_back(0.0f);

						// 仮color
						meshData.vertex.push_back(1.0f);
						meshData.vertex.push_back(1.0f);
						meshData.vertex.push_back(1.0f);
						meshData.vertex.push_back(1.0f);

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
					}
				}

				//MyosotisFW::Meshlet currentMeshletData{};
				//std::unordered_set<uint32_t> currentUniqueIndex;
				//bool firstDataForMeshletAABB = true;
				//for (uint32_t polygonIdx = 0; polygonIdx < geometryPartition.polygon_count; polygonIdx++)
				//{
				//	const ofbx::GeometryPartition::Polygon& polygon = geometryPartition.polygons[polygonIdx];
				//	std::vector<uint32_t> triangle{};
				//	triangulate(geomData, polygon, triangle);
				//	glm::vec3 v0 = glm::vec3(positions.get(triangle[0]).x, positions.get(triangle[0]).y, positions.get(triangle[0]).z);
				//	glm::vec3 v1 = glm::vec3(positions.get(triangle[1]).x, positions.get(triangle[1]).y, positions.get(triangle[1]).z);
				//	glm::vec3 v2 = glm::vec3(positions.get(triangle[2]).x, positions.get(triangle[2]).y, positions.get(triangle[2]).z);

				//	// 頂点追加後のサイズ
				//	size_t newUnique = currentMeshletData.uniqueIndex.size();
				//	for (uint32_t i = 0; i < 3; i++)
				//	{
				//		if (currentUniqueIndex.find(triangle[i]) == currentUniqueIndex.end())
				//		{
				//			newUnique++;
				//		}
				//	}

				//	// 制限チェック
				//	if ((newUnique >= MyosotisFW::AppInfo::g_maxMeshletVertices) ||
				//		((currentMeshletData.primitives.size() / 3) + 3 >= MyosotisFW::AppInfo::g_maxMeshletPrimitives))
				//	{
				//		meshData.meshlet.push_back(currentMeshletData);
				//		currentMeshletData = MyosotisFW::Meshlet();
				//		currentUniqueIndex.clear();
				//		firstDataForMeshletAABB = true;
				//	}

				//	// 頂点追加
				//	for (uint32_t i = 0; i < 3; i++)
				//	{
				//		if (currentUniqueIndex.insert(triangle[i]).second)
				//		{
				//			currentMeshletData.uniqueIndex.push_back(triangle[i]);
				//		}
				//	}

				//	// AABB更新
				//	if (firstDataForMeshletAABB)
				//	{
				//		currentMeshletData.meshletInfo.AABBMin = glm::vec4(glm::min(v0, glm::min(v1, v2)), 0.0f);
				//		currentMeshletData.meshletInfo.AABBMax = glm::vec4(glm::max(v0, glm::max(v1, v2)), 0.0f);
				//		firstDataForMeshletAABB = false;
				//	}
				//	else
				//	{
				//		currentMeshletData.meshletInfo.AABBMin = glm::min(currentMeshletData.meshletInfo.AABBMin, glm::min(glm::vec4(v0, 0.0f), glm::min(glm::vec4(v1, 0.0f), glm::vec4(v2, 0.0f))));
				//		currentMeshletData.meshletInfo.AABBMax = glm::max(currentMeshletData.meshletInfo.AABBMax, glm::max(glm::vec4(v0, 0.0f), glm::max(glm::vec4(v1, 0.0f), glm::vec4(v2, 0.0f))));
				//	}

				//	// 三角形追加
				//	auto tri1 = std::find(currentMeshletData.uniqueIndex.begin(), currentMeshletData.uniqueIndex.end(), triangle[0]);
				//	size_t index1 = std::distance(currentMeshletData.uniqueIndex.begin(), tri1);
				//	auto tri2 = std::find(currentMeshletData.uniqueIndex.begin(), currentMeshletData.uniqueIndex.end(), triangle[1]);
				//	size_t index2 = std::distance(currentMeshletData.uniqueIndex.begin(), tri2);
				//	auto tri3 = std::find(currentMeshletData.uniqueIndex.begin(), currentMeshletData.uniqueIndex.end(), triangle[2]);
				//	size_t index3 = std::distance(currentMeshletData.uniqueIndex.begin(), tri3);
				//	currentMeshletData.primitives.push_back(index1);
				//	currentMeshletData.primitives.push_back(index2);
				//	currentMeshletData.primitives.push_back(index3);
				//}
				//if (!currentMeshletData.primitives.empty())
				//{
				//	meshData.meshlet.push_back(currentMeshletData);
				//}

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

				// メッシュレット数の上限
				size_t maxMeshlets = meshopt_buildMeshletsBound(
					index.size(),
					MyosotisFW::AppInfo::g_maxMeshletVertices,
					MyosotisFW::AppInfo::g_maxMeshletPrimitives);

				// meshoptimizer用データ作成
				std::vector<meshopt_Meshlet> meshlets(maxMeshlets);
				std::vector<uint32_t> meshletVertices(maxMeshlets * MyosotisFW::AppInfo::g_maxMeshletVertices);
				std::vector<uint8_t> meshletTriangles(maxMeshlets * MyosotisFW::AppInfo::g_maxMeshletPrimitives * 3);

				// メッシュレット生成
				size_t meshletCount = meshopt_buildMeshlets(
					meshlets.data(),
					meshletVertices.data(),
					meshletTriangles.data(),
					index.data(),
					index.size(),
					&tmpPositions[0].x,
					tmpPositions.size(),
					sizeof(glm::vec3),
					MyosotisFW::AppInfo::g_maxMeshletVertices,
					MyosotisFW::AppInfo::g_maxMeshletPrimitives,
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
					dst.primitives.reserve(src.triangle_count * 3);
					for (size_t t = 0; t < src.triangle_count * 3; t++)
					{
						uint32_t triangleIndex = static_cast<uint32_t>(meshletTriangles[src.triangle_offset + t]);
						dst.primitives.push_back(triangleIndex);
					}

					// AABB
					glm::vec3 p0 = tmpPositions[meshletVertices[src.vertex_offset + 0]];
					dst.meshletInfo.AABBMin = glm::vec4(p0, 0.0f);
					dst.meshletInfo.AABBMax = glm::vec4(p0, 0.0f);
					for (size_t v = 1; v < src.vertex_count; v++)
					{
						uint32_t vertexIndex = meshletVertices[src.vertex_offset + v];
						const glm::vec3& pos = tmpPositions[vertexIndex];
						dst.meshletInfo.AABBMin = glm::min(dst.meshletInfo.AABBMin, glm::vec4(pos, 0.0f));
						dst.meshletInfo.AABBMax = glm::max(dst.meshletInfo.AABBMax, glm::vec4(pos, 0.0f));
					}

					dst.meshletInfo.vertexCount = src.vertex_count;
					dst.meshletInfo.primitiveCount = src.triangle_count;

					meshData.meshlet.push_back(dst);
				}
				meshes.push_back(meshData);
			}
			scene->destroy();
#ifdef DEBUG
			Logger::Debug("[VK_Loader] End load: " + fileName +
				"(" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count()) + "ms)");
#endif
			return meshes;
		}
	}
}