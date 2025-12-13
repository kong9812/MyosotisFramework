// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include <fstream>
#include <vector>
#include <chrono>
#include <unordered_set>

#include "AppInfo.h"
#include "Logger.h"
#include "Mesh.h"

#include "itiny_gltf.h"

#include "imeshoptimizer.h"
#include "ixatlas.h"

namespace Utility::Loader {
	inline const float* GetGLTFFloatData(const tinygltf::Model& glTFModel, const tinygltf::Primitive& primitive, const char* attributeName, size_t* count = nullptr)
	{
		if (primitive.attributes.find(attributeName) == primitive.attributes.end()) return nullptr;

		const tinygltf::Accessor& accessor = glTFModel.accessors[primitive.attributes.find(attributeName)->second];
		const tinygltf::BufferView& view = glTFModel.bufferViews[accessor.bufferView];
		if (count) *count = accessor.count;
		return reinterpret_cast<const float*>(&(glTFModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
	}

	inline void AddIndex(
		const float* positionBuffer,
		const uint32_t triangle[3],
		MyosotisFW::Mesh& meshData,
		MyosotisFW::Meshlet& currentMeshletData,
		std::unordered_set<uint32_t>& currentUniqueIndex,
		bool& firstDataForMeshletAABB)
	{
		glm::vec3 v0 = glm::make_vec3(&positionBuffer[triangle[0] * 3]);
		glm::vec3 v1 = glm::make_vec3(&positionBuffer[triangle[1] * 3]);
		glm::vec3 v2 = glm::make_vec3(&positionBuffer[triangle[2] * 3]);

		// 頂点追加後のサイズ
		size_t newUnique = currentMeshletData.uniqueIndex.size();
		for (uint32_t i = 0; i < 3; i++)
		{
			if (currentUniqueIndex.find(triangle[i]) == currentUniqueIndex.end())
			{
				newUnique++;
			}
		}

		// 制限チェック
		if ((newUnique >= MyosotisFW::AppInfo::g_maxMeshletVertices) ||
			((currentMeshletData.primitives.size() / 3) + 3 >= MyosotisFW::AppInfo::g_maxMeshletPrimitives))
		{
			meshData.meshlet.push_back(currentMeshletData);
			currentMeshletData = MyosotisFW::Meshlet();
			currentUniqueIndex.clear();
			firstDataForMeshletAABB = true;
		}

		// 頂点追加
		for (uint32_t i = 0; i < 3; i++)
		{
			if (currentUniqueIndex.insert(triangle[i]).second)
			{
				currentMeshletData.uniqueIndex.push_back(triangle[i]);
			}
		}

		// AABB更新
		if (firstDataForMeshletAABB)
		{
			currentMeshletData.meshletInfo.AABBMin = glm::vec4(glm::min(v0, glm::min(v1, v2)), 0.0f);
			currentMeshletData.meshletInfo.AABBMax = glm::vec4(glm::max(v0, glm::max(v1, v2)), 0.0f);
			firstDataForMeshletAABB = false;
		}
		else
		{
			currentMeshletData.meshletInfo.AABBMin = glm::min(currentMeshletData.meshletInfo.AABBMin, glm::min(glm::vec4(v0, 0.0f), glm::min(glm::vec4(v1, 0.0f), glm::vec4(v2, 0.0f))));
			currentMeshletData.meshletInfo.AABBMax = glm::max(currentMeshletData.meshletInfo.AABBMax, glm::max(glm::vec4(v0, 0.0f), glm::max(glm::vec4(v1, 0.0f), glm::vec4(v2, 0.0f))));
		}

		// 三角形追加
		auto tri1 = std::find(currentMeshletData.uniqueIndex.begin(), currentMeshletData.uniqueIndex.end(), triangle[0]);
		size_t index1 = std::distance(currentMeshletData.uniqueIndex.begin(), tri1);
		auto tri2 = std::find(currentMeshletData.uniqueIndex.begin(), currentMeshletData.uniqueIndex.end(), triangle[1]);
		size_t index2 = std::distance(currentMeshletData.uniqueIndex.begin(), tri2);
		auto tri3 = std::find(currentMeshletData.uniqueIndex.begin(), currentMeshletData.uniqueIndex.end(), triangle[2]);
		size_t index3 = std::distance(currentMeshletData.uniqueIndex.begin(), tri3);
		currentMeshletData.primitives.push_back(index1);
		currentMeshletData.primitives.push_back(index2);
		currentMeshletData.primitives.push_back(index3);
	}

	inline std::vector<MyosotisFW::Mesh> loadGltf(std::string fileName)
	{
#ifdef DEBUG
		Logger::Debug("[VK_Loader] Start load: " + fileName);
		auto start = std::chrono::high_resolution_clock::now();
#endif
		tinygltf::Model glTFModel{};
		tinygltf::TinyGLTF glTFLoader{};
		std::string error{};
		std::string warning{};

		bool fileLoaded = glTFLoader.LoadASCIIFromFile(&glTFModel, &error, &warning, std::string(MyosotisFW::AppInfo::g_modelFolder) + fileName);
		ASSERT(fileLoaded, "Failed to open gltf file: " + std::string(MyosotisFW::AppInfo::g_modelFolder) + fileName + "\nerror: " + error);

		std::vector<MyosotisFW::Mesh> meshes{};
		for (const tinygltf::Mesh& mesh : glTFModel.meshes)
		{
			for (const tinygltf::Primitive& primitive : mesh.primitives)
			{
				MyosotisFW::Mesh meshData{};
				bool firstDataForAABB = true;
				size_t vertexCount = 0;
				const float* positionBuffer = GetGLTFFloatData(glTFModel, primitive, "POSITION", &vertexCount);
				const float* normalBuffer = GetGLTFFloatData(glTFModel, primitive, "NORMAL");
				const float* uvBuffer = GetGLTFFloatData(glTFModel, primitive, "TEXCOORD_0");
				const float* colorBuffer = GetGLTFFloatData(glTFModel, primitive, "COLOR_0");

				// add vertex data
				for (size_t vertex = 0; vertex < vertexCount; vertex++)
				{
					glm::vec3 v = glm::vec3(0.0f);
					glm::vec3 n = glm::vec3(0.0f);
					glm::vec2 u0 = glm::vec2(0.0f);
					glm::vec2 u1 = glm::vec2(0.0f);
					glm::vec4 c = glm::vec4(1.0f);

					if (positionBuffer != nullptr)
					{
						v = glm::make_vec3(&positionBuffer[vertex * 3]);

						// aabb
						if (firstDataForAABB)
						{
							meshData.meshInfo.AABBMin = glm::vec4(v, 0.0f);
							meshData.meshInfo.AABBMax = glm::vec4(v, 0.0f);
							firstDataForAABB = false;
						}
						else
						{
							meshData.meshInfo.AABBMin = glm::min(meshData.meshInfo.AABBMin, glm::vec4(v, 0.0f));
							meshData.meshInfo.AABBMax = glm::max(meshData.meshInfo.AABBMax, glm::vec4(v, 0.0f));
						}
					}

					if (normalBuffer != nullptr)
					{
						n = glm::normalize(glm::make_vec3(&normalBuffer[vertex * 3]));
					}


					if (uvBuffer != nullptr)
					{
						u0 = glm::make_vec2(&uvBuffer[vertex * 2]);
					}

					if (colorBuffer != nullptr)
					{
						c = glm::vec4(glm::make_vec3(&colorBuffer[vertex * 3]), 1.0f);
					}

					meshData.vertex.insert(meshData.vertex.end(), { v, n, u0, u1, c });
				}

				// Index
				std::vector<uint32_t> index{};
				const tinygltf::Accessor& accessor = glTFModel.accessors[primitive.indices];
				const tinygltf::BufferView& view = glTFModel.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = glTFModel.buffers[view.buffer];
				const uint32_t trianglesCount = accessor.count / 3;
				switch (accessor.componentType) {
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
					const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + view.byteOffset]);
					for (size_t primitiveIndex = 0; primitiveIndex < trianglesCount; primitiveIndex++) {
						const uint32_t triangle[3] = {
							static_cast<uint32_t>(buf[primitiveIndex * 3 + 0]),
							static_cast<uint32_t>(buf[primitiveIndex * 3 + 1]),
							static_cast<uint32_t>(buf[primitiveIndex * 3 + 2])
						};
						index.insert(index.end(), { triangle[0], triangle[1], triangle[2] });
					}
					break;
				}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
					const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + view.byteOffset]);
					for (size_t primitiveIndex = 0; primitiveIndex < trianglesCount; primitiveIndex++) {
						const uint32_t triangle[3] = {
							static_cast<uint32_t>(buf[primitiveIndex * 3 + 0]),
							static_cast<uint32_t>(buf[primitiveIndex * 3 + 1]),
							static_cast<uint32_t>(buf[primitiveIndex * 3 + 2])
						};
						index.insert(index.end(), { triangle[0], triangle[1], triangle[2] });
					}
					break;
				}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
					const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + view.byteOffset]);
					for (size_t primitiveIndex = 0; primitiveIndex < trianglesCount; primitiveIndex++) {
						const uint32_t triangle[3] = {
							static_cast<uint32_t>(buf[primitiveIndex * 3 + 0]),
							static_cast<uint32_t>(buf[primitiveIndex * 3 + 1]),
							static_cast<uint32_t>(buf[primitiveIndex * 3 + 2])
						};
						index.insert(index.end(), { triangle[0], triangle[1], triangle[2] });
					}
					break;
				}
				default:
					ASSERT(false, "Index component type not supported! File: " + fileName);
					break;
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
		}
#ifdef DEBUG
		Logger::Debug("[VK_Loader] End load: " + fileName +
			"(" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count()) + "ms)");
#endif
		return meshes;
	}
}
