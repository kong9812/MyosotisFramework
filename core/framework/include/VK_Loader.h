// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include <fstream>
#include <vector>
#include <chrono>
#include <unordered_set>

#include "iRapidJson.h"
#include "iofbx.h"
#include "istb_image.h"
#include "itiny_gltf.h"
#include "AppInfo.h"

#include "Logger.h"
#include "AppInfo.h"
#include "Structs.h"
#include "VK_CreateInfo.h"
#include "VK_Validation.h"
#include "RenderQueue.h"

namespace Utility::Loader {
	inline VkShaderModule loadShader(VkDevice device, std::string fileName, const VkAllocationCallbacks* pAllocator = nullptr)
	{
#ifdef DEBUG
		Logger::Debug("[VK_Loader] Start load: " + fileName);
		auto start = std::chrono::high_resolution_clock::now();
#endif
		std::filesystem::path absolutePath = std::filesystem::absolute(MyosotisFW::AppInfo::g_shaderFolder + fileName);

		std::ifstream file(absolutePath.string().c_str(), std::ios::ate | std::ios::binary);
		ASSERT(file.is_open(), "Failed to open shader file: " + std::string(MyosotisFW::AppInfo::g_shaderFolder) + fileName);

		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buf(fileSize);
		file.seekg(0);
		file.read(buf.data(), fileSize);
		file.close();

		VkShaderModuleCreateInfo shaderModuleCreateInfo{};
		shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCreateInfo.codeSize = buf.size();
		shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(buf.data());

		VkShaderModule shaderModule{};
		VK_VALIDATION(vkCreateShaderModule(device, &shaderModuleCreateInfo, pAllocator, &shaderModule));
#ifdef DEBUG
		Logger::Debug("[VK_Loader] End load: " + fileName +
			"(" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count()) + "ms)");
#endif
		return shaderModule;
	}

	inline rapidjson::Document loadGameStageFile(std::string fileName)
	{
		std::ifstream file(MyosotisFW::AppInfo::g_gameStageFolder + fileName);
		ASSERT(file.is_open(), "Failed to open game stage file: " + std::string(MyosotisFW::AppInfo::g_gameStageFolder) + fileName);
		rapidjson::IStreamWrapper istream(file);
		rapidjson::Document doc{};
		doc.ParseStream(istream);
		file.close();
		return doc;
	}
	inline void saveGameStageFile(std::string fileName, rapidjson::Document& doc)
	{
		rapidjson::StringBuffer buffer{};
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		doc.Accept(writer);
		std::ofstream file(MyosotisFW::AppInfo::g_gameStageFolder + fileName, std::ios::trunc);
		ASSERT(file.is_open(), "Failed to open game stage file: " + std::string(MyosotisFW::AppInfo::g_gameStageFolder) + fileName);
		file << buffer.GetString();
		file.close();
	}

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
				const float* positionBuffer = nullptr;
				const float* normalBuffer = nullptr;
				const float* uvBuffer = nullptr;
				const float* colorBuffer = nullptr;

				size_t vertexCount = 0;
				// position
				if (primitive.attributes.find("POSITION") != primitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = glTFModel.accessors[primitive.attributes.find("POSITION")->second];
					const tinygltf::BufferView& view = glTFModel.bufferViews[accessor.bufferView];
					positionBuffer = reinterpret_cast<const float*>(&(glTFModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					vertexCount = accessor.count;
				}

				// normal
				if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = glTFModel.accessors[primitive.attributes.find("NORMAL")->second];
					const tinygltf::BufferView& view = glTFModel.bufferViews[accessor.bufferView];
					normalBuffer = reinterpret_cast<const float*>(&(glTFModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
				}

				// uv
				if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = glTFModel.accessors[primitive.attributes.find("TEXCOORD_0")->second];
					const tinygltf::BufferView& view = glTFModel.bufferViews[accessor.bufferView];
					uvBuffer = reinterpret_cast<const float*>(&(glTFModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
				}

				// color
				if (primitive.attributes.find("COLOR_0") != primitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = glTFModel.accessors[primitive.attributes.find("COLOR_0")->second];
					const tinygltf::BufferView& view = glTFModel.bufferViews[accessor.bufferView];
					colorBuffer = reinterpret_cast<const float*>(&(glTFModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
				}

				// add vertex data
				for (size_t vertex = 0; vertex < vertexCount; vertex++)
				{
					if (positionBuffer != nullptr)
					{
						glm::vec3 v = glm::make_vec3(&positionBuffer[vertex * 3]);
						meshData.vertex.push_back(v.x);
						meshData.vertex.push_back(v.y);
						meshData.vertex.push_back(v.z);
						meshData.vertex.push_back(1.0f);

						// aabb
						if (firstDataForAABB)
						{
							meshData.min.x = positionBuffer ? positionBuffer[vertex * 3 + 0] : 0.0f;
							meshData.min.y = positionBuffer ? positionBuffer[vertex * 3 + 1] : 0.0f;
							meshData.min.z = positionBuffer ? positionBuffer[vertex * 3 + 2] : 0.0f;
							meshData.max.x = meshData.min.x;
							meshData.max.y = meshData.min.y;
							meshData.max.z = meshData.min.z;
							firstDataForAABB = false;
						}
						else
						{
							if (positionBuffer)
							{
								meshData.min.x = meshData.min.x < v.x ? meshData.min.x : v.x;
								meshData.min.y = meshData.min.y < v.y ? meshData.min.y : v.y;
								meshData.min.z = meshData.min.z < v.z ? meshData.min.z : v.z;
								meshData.max.x = meshData.max.x > v.x ? meshData.max.x : v.x;
								meshData.max.y = meshData.max.y > v.y ? meshData.max.y : v.y;
								meshData.max.z = meshData.max.z > v.z ? meshData.max.z : v.z;
							}
						}
					}
					else
					{
						// どうかなぁ…
						meshData.vertex.push_back(0.0f);
						meshData.vertex.push_back(0.0f);
						meshData.vertex.push_back(0.0f);
						meshData.vertex.push_back(1.0f);
					}

					if (normalBuffer != nullptr)
					{
						glm::vec3 n = glm::normalize(glm::make_vec3(&normalBuffer[vertex * 3]));
						meshData.vertex.push_back(n.x);
						meshData.vertex.push_back(n.y);
						meshData.vertex.push_back(n.z);
					}
					else
					{
						meshData.vertex.push_back(0.0f);
						meshData.vertex.push_back(0.0f);
						meshData.vertex.push_back(0.0f);
					}

					if (uvBuffer != nullptr)
					{
						glm::vec2 uv = glm::make_vec2(&uvBuffer[vertex * 2]);
						meshData.vertex.push_back(uv.x);
						meshData.vertex.push_back(uv.y);
					}
					else
					{
						meshData.vertex.push_back(0.0f);
						meshData.vertex.push_back(0.0f);
					}

					if (colorBuffer != nullptr)
					{
						glm::vec3 color = glm::make_vec3(&colorBuffer[vertex * 3]);
						meshData.vertex.push_back(color.r);
						meshData.vertex.push_back(color.g);
						meshData.vertex.push_back(color.b);
						meshData.vertex.push_back(1.0f);
					}
					else
					{
						meshData.vertex.push_back(1.0f);
						meshData.vertex.push_back(1.0f);
						meshData.vertex.push_back(1.0f);
						meshData.vertex.push_back(1.0f);
					}
				}
				{// add index data
					const tinygltf::Accessor& accessor = glTFModel.accessors[primitive.indices];
					const tinygltf::BufferView& view = glTFModel.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = glTFModel.buffers[view.buffer];

					const uint32_t trianglesCount = accessor.count / 3;
					MyosotisFW::Meshlet currentMeshletData{};
					std::unordered_set<uint32_t> currentUniqueIndex;
					bool firstDataForMeshletAABB = true;

					// glTF supports different component types of indices
					switch (accessor.componentType) {
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
						const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + view.byteOffset]);
						for (size_t primitiveIndex = 0; primitiveIndex < trianglesCount; primitiveIndex++) {
							const uint32_t triangle[3] = {
								buf[primitiveIndex * 3 + 0],
								buf[primitiveIndex * 3 + 1],
								buf[primitiveIndex * 3 + 2]
							};

							// 頂点追加後のサイズ
							size_t newUnique = currentUniqueIndex.size();
							for (uint32_t i = 0; i < 3; i++)
							{
								if (currentUniqueIndex.find(triangle[i]) == currentUniqueIndex.end())
								{
									newUnique++;
								}
							}

							// 制限チェック
							if ((currentUniqueIndex.size() + newUnique > MyosotisFW::AppInfo::g_maxMeshletVertices) ||
								(currentMeshletData.primitives.size() + 3 >= MyosotisFW::AppInfo::g_maxMeshletPrimitives))
							{
								currentMeshletData.uniqueIndex.insert(currentMeshletData.uniqueIndex.end(), currentUniqueIndex.begin(), currentUniqueIndex.end());
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
								currentMeshletData.min.x = positionBuffer[triangle[0]];
								currentMeshletData.min.y = positionBuffer[triangle[1]];
								currentMeshletData.min.z = positionBuffer[triangle[2]];
								currentMeshletData.max.x = positionBuffer[triangle[0]];
								currentMeshletData.max.y = positionBuffer[triangle[1]];
								currentMeshletData.max.z = positionBuffer[triangle[2]];
								firstDataForMeshletAABB = false;
							}
							else
							{
								currentMeshletData.min.x = currentMeshletData.min.x < positionBuffer[triangle[0]] ? currentMeshletData.min.x : positionBuffer[triangle[0]];
								currentMeshletData.min.y = currentMeshletData.min.y < positionBuffer[triangle[1]] ? currentMeshletData.min.y : positionBuffer[triangle[1]];
								currentMeshletData.min.z = currentMeshletData.min.z < positionBuffer[triangle[2]] ? currentMeshletData.min.z : positionBuffer[triangle[2]];
								currentMeshletData.max.x = currentMeshletData.max.x > positionBuffer[triangle[0]] ? currentMeshletData.max.x : positionBuffer[triangle[0]];
								currentMeshletData.max.y = currentMeshletData.max.y > positionBuffer[triangle[1]] ? currentMeshletData.max.y : positionBuffer[triangle[1]];
								currentMeshletData.max.z = currentMeshletData.max.z > positionBuffer[triangle[2]] ? currentMeshletData.max.z : positionBuffer[triangle[2]];
							}

							// 三角形追加
							currentMeshletData.primitives.push_back(triangle[0]);
							currentMeshletData.primitives.push_back(triangle[1]);
							currentMeshletData.primitives.push_back(triangle[2]);
						}
						if (!currentMeshletData.primitives.empty())
						{
							meshData.meshlet.push_back(currentMeshletData);
						}
						break;
					}
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
						const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + view.byteOffset]);
						for (size_t primitiveIndex = 0; primitiveIndex < trianglesCount; primitiveIndex++) {
							const uint16_t triangle[3] = {
								buf[primitiveIndex * 3 + 0],
								buf[primitiveIndex * 3 + 1],
								buf[primitiveIndex * 3 + 2]
							};

							// 頂点追加後のサイズ
							size_t newUnique = currentUniqueIndex.size();
							for (uint32_t i = 0; i < 3; i++)
							{
								if (currentUniqueIndex.find(triangle[i]) == currentUniqueIndex.end())
								{
									newUnique++;
								}
							}

							// 制限チェック
							if ((currentUniqueIndex.size() + newUnique > MyosotisFW::AppInfo::g_maxMeshletVertices) ||
								(currentMeshletData.primitives.size() + 3 >= MyosotisFW::AppInfo::g_maxMeshletPrimitives))
							{
								currentMeshletData.uniqueIndex.insert(currentMeshletData.uniqueIndex.end(), currentUniqueIndex.begin(), currentUniqueIndex.end());
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
								currentMeshletData.min.x = positionBuffer[triangle[0]];
								currentMeshletData.min.y = positionBuffer[triangle[1]];
								currentMeshletData.min.z = positionBuffer[triangle[2]];
								currentMeshletData.max.x = positionBuffer[triangle[0]];
								currentMeshletData.max.y = positionBuffer[triangle[1]];
								currentMeshletData.max.z = positionBuffer[triangle[2]];
								firstDataForMeshletAABB = false;
							}
							else
							{
								currentMeshletData.min.x = currentMeshletData.min.x < positionBuffer[triangle[0]] ? currentMeshletData.min.x : positionBuffer[triangle[0]];
								currentMeshletData.min.y = currentMeshletData.min.y < positionBuffer[triangle[1]] ? currentMeshletData.min.y : positionBuffer[triangle[1]];
								currentMeshletData.min.z = currentMeshletData.min.z < positionBuffer[triangle[2]] ? currentMeshletData.min.z : positionBuffer[triangle[2]];
								currentMeshletData.max.x = currentMeshletData.max.x > positionBuffer[triangle[0]] ? currentMeshletData.max.x : positionBuffer[triangle[0]];
								currentMeshletData.max.y = currentMeshletData.max.y > positionBuffer[triangle[1]] ? currentMeshletData.max.y : positionBuffer[triangle[1]];
								currentMeshletData.max.z = currentMeshletData.max.z > positionBuffer[triangle[2]] ? currentMeshletData.max.z : positionBuffer[triangle[2]];
							}

							// 三角形追加
							currentMeshletData.primitives.push_back(triangle[0]);
							currentMeshletData.primitives.push_back(triangle[1]);
							currentMeshletData.primitives.push_back(triangle[2]);
						}
						if (!currentMeshletData.primitives.empty())
						{
							meshData.meshlet.push_back(currentMeshletData);
						}
						break;
					}
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
						const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + view.byteOffset]);
						for (size_t primitiveIndex = 0; primitiveIndex < trianglesCount; primitiveIndex++) {
							const uint8_t triangle[3] = {
								buf[primitiveIndex * 3 + 0],
								buf[primitiveIndex * 3 + 1],
								buf[primitiveIndex * 3 + 2]
							};

							// 頂点追加後のサイズ
							size_t newUnique = currentUniqueIndex.size();
							for (uint32_t i = 0; i < 3; i++)
							{
								if (currentUniqueIndex.find(triangle[i]) == currentUniqueIndex.end())
								{
									newUnique++;
								}
							}

							// 制限チェック
							if ((currentUniqueIndex.size() + newUnique > MyosotisFW::AppInfo::g_maxMeshletVertices) ||
								(currentMeshletData.primitives.size() + 3 >= MyosotisFW::AppInfo::g_maxMeshletPrimitives))
							{
								currentMeshletData.uniqueIndex.insert(currentMeshletData.uniqueIndex.end(), currentUniqueIndex.begin(), currentUniqueIndex.end());
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
								currentMeshletData.min.x = positionBuffer[triangle[0]];
								currentMeshletData.min.y = positionBuffer[triangle[1]];
								currentMeshletData.min.z = positionBuffer[triangle[2]];
								currentMeshletData.max.x = positionBuffer[triangle[0]];
								currentMeshletData.max.y = positionBuffer[triangle[1]];
								currentMeshletData.max.z = positionBuffer[triangle[2]];
								firstDataForMeshletAABB = false;
							}
							else
							{
								currentMeshletData.min.x = currentMeshletData.min.x < positionBuffer[triangle[0]] ? currentMeshletData.min.x : positionBuffer[triangle[0]];
								currentMeshletData.min.y = currentMeshletData.min.y < positionBuffer[triangle[1]] ? currentMeshletData.min.y : positionBuffer[triangle[1]];
								currentMeshletData.min.z = currentMeshletData.min.z < positionBuffer[triangle[2]] ? currentMeshletData.min.z : positionBuffer[triangle[2]];
								currentMeshletData.max.x = currentMeshletData.max.x > positionBuffer[triangle[0]] ? currentMeshletData.max.x : positionBuffer[triangle[0]];
								currentMeshletData.max.y = currentMeshletData.max.y > positionBuffer[triangle[1]] ? currentMeshletData.max.y : positionBuffer[triangle[1]];
								currentMeshletData.max.z = currentMeshletData.max.z > positionBuffer[triangle[2]] ? currentMeshletData.max.z : positionBuffer[triangle[2]];
							}

							// 三角形追加
							currentMeshletData.primitives.push_back(triangle[0]);
							currentMeshletData.primitives.push_back(triangle[1]);
							currentMeshletData.primitives.push_back(triangle[2]);
						}
						if (!currentMeshletData.primitives.empty())
						{
							meshData.meshlet.push_back(currentMeshletData);
						}
						break;
					}
					default:
						ASSERT(false, "Index component type not supported! File: " + fileName);
						break;
					}
				}
				meshes.push_back(meshData);
			}
		}
#ifdef DEBUG
		Logger::Debug("[VK_Loader] End load: " + fileName +
			"(" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count()) + "ms)");
#endif
		return meshes;
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

		uint32_t indicesOffset = 0;
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
							meshData.min.x = v.x;
							meshData.min.y = v.y;
							meshData.min.z = v.z;
							meshData.max.x = v.x;
							meshData.max.y = v.y;
							meshData.max.z = v.z;
							firstDataForAABB = false;
						}
						else
						{
							meshData.min.x = meshData.min.x < v.x ? meshData.min.x : v.x;
							meshData.min.y = meshData.min.y < v.y ? meshData.min.y : v.y;
							meshData.min.z = meshData.min.z < v.z ? meshData.min.z : v.z;
							meshData.max.x = meshData.max.x > v.x ? meshData.max.x : v.x;
							meshData.max.y = meshData.max.y > v.y ? meshData.max.y : v.y;
							meshData.max.z = meshData.max.z > v.z ? meshData.max.z : v.z;
						}
					}
				}

				MyosotisFW::Meshlet currentMeshletData{};
				std::unordered_set<uint32_t> currentUniqueIndex;
				bool firstDataForMeshletAABB = true;
				for (uint32_t polygonIdx = 0; polygonIdx < geometryPartition.polygon_count; polygonIdx++)
				{
					const ofbx::GeometryPartition::Polygon& polygon = geometryPartition.polygons[polygonIdx];
					std::vector<uint32_t> triangle{};
					triangulate(geomData, polygon, triangle);

					// 頂点追加後のサイズ
					size_t newUnique = currentUniqueIndex.size();
					for (uint32_t i = 0; i < 3; i++)
					{
						if (currentUniqueIndex.find(triangle[i]) == currentUniqueIndex.end())
						{
							newUnique++;
						}
					}

					// 制限チェック
					if ((currentUniqueIndex.size() + newUnique > MyosotisFW::AppInfo::g_maxMeshletVertices) ||
						(currentMeshletData.primitives.size() + 3 >= MyosotisFW::AppInfo::g_maxMeshletPrimitives))
					{
						currentMeshletData.uniqueIndex.insert(currentMeshletData.uniqueIndex.end(), currentUniqueIndex.begin(), currentUniqueIndex.end());
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
						currentMeshletData.min.x = positions.get(triangle[0]).x;
						currentMeshletData.min.y = positions.get(triangle[1]).y;
						currentMeshletData.min.z = positions.get(triangle[2]).z;
						currentMeshletData.max.x = positions.get(triangle[0]).x;
						currentMeshletData.max.y = positions.get(triangle[1]).y;
						currentMeshletData.max.z = positions.get(triangle[2]).z;
						firstDataForMeshletAABB = false;
					}
					else
					{
						currentMeshletData.min.x = currentMeshletData.min.x < positions.get(triangle[0]).x ? currentMeshletData.min.x : positions.get(triangle[0]).x;
						currentMeshletData.min.y = currentMeshletData.min.y < positions.get(triangle[1]).y ? currentMeshletData.min.y : positions.get(triangle[1]).y;
						currentMeshletData.min.z = currentMeshletData.min.z < positions.get(triangle[2]).z ? currentMeshletData.min.z : positions.get(triangle[2]).z;
						currentMeshletData.max.x = currentMeshletData.max.x > positions.get(triangle[0]).x ? currentMeshletData.max.x : positions.get(triangle[0]).x;
						currentMeshletData.max.y = currentMeshletData.max.y > positions.get(triangle[1]).y ? currentMeshletData.max.y : positions.get(triangle[1]).y;
						currentMeshletData.max.z = currentMeshletData.max.z > positions.get(triangle[2]).z ? currentMeshletData.max.z : positions.get(triangle[2]).z;
					}

					// 三角形追加
					currentMeshletData.primitives.push_back(triangle[0]);
					currentMeshletData.primitives.push_back(triangle[1]);
					currentMeshletData.primitives.push_back(triangle[2]);
				}
				if (!currentMeshletData.primitives.empty())
				{
					currentMeshletData.uniqueIndex.insert(currentMeshletData.uniqueIndex.end(), currentUniqueIndex.begin(), currentUniqueIndex.end());
					meshData.meshlet.push_back(currentMeshletData);
				}
				indicesOffset += positions.count;
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

	inline MyosotisFW::VMAImage loadImage(VkDevice device, MyosotisFW::System::Render::RenderQueue_ptr queue, VkCommandPool commandPool, VmaAllocator allocator, std::string fileName, const VkAllocationCallbacks* pAllocationCallbacks = nullptr)
	{
		MyosotisFW::VMAImage image{};

		std::filesystem::path absolutePath = std::filesystem::absolute(MyosotisFW::AppInfo::g_textureFolder + fileName);

		// loadImage fence
		VkFence fence = VK_NULL_HANDLE;
		VkFenceCreateInfo fenceCreateInfo = Utility::Vulkan::CreateInfo::fenceCreateInfo();
		VK_VALIDATION(vkCreateFence(device, &fenceCreateInfo, pAllocationCallbacks, &fence));

		int textureWidth = -1;
		int textureHeight = -1;
		int textureChannels = -1;
		stbi_uc* pixels = stbi_load(absolutePath.string().c_str(), &textureWidth, &textureHeight, &textureChannels, STBI_rgb_alpha);
		ASSERT(pixels, "Failed to load image: " + absolutePath.string());
		size_t imageSize = textureWidth * textureHeight * textureChannels;

		{// GPU image
			VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfo(VkFormat::VK_FORMAT_R8G8B8A8_SRGB, textureWidth, textureHeight);
			VmaAllocationCreateInfo allocationCreateInfo{};
			allocationCreateInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;
			VK_VALIDATION(vmaCreateImage(allocator, &imageCreateInfo, &allocationCreateInfo, &image.image, &image.allocation, &image.allocationInfo));
			VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfo(image.image, VkFormat::VK_FORMAT_R8G8B8A8_SRGB);
			VK_VALIDATION(vkCreateImageView(device, &imageViewCreateInfo, pAllocationCallbacks, &image.view));
		}

		MyosotisFW::Buffer stagingBuffer{};
		{// CPU buffer (staging buffer)
			VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(imageSize, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
			VmaAllocationCreateInfo allocationCreateInfo{};
			allocationCreateInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY;
			VK_VALIDATION(vmaCreateBuffer(allocator, &bufferCreateInfo, &allocationCreateInfo, &stagingBuffer.buffer, &stagingBuffer.allocation, &stagingBuffer.allocationInfo));
		}

		{// CPU -> GPU
			VkCommandBuffer commandBuffer{};
			VkCommandBufferAllocateInfo commandBufferAllocateInfo = Utility::Vulkan::CreateInfo::commandBufferAllocateInfo(commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
			VK_VALIDATION(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer));

			void* data{};
			VK_VALIDATION(vmaMapMemory(allocator, stagingBuffer.allocation, &data));
			memcpy(data, pixels, imageSize);
			vmaUnmapMemory(allocator, stagingBuffer.allocation);
			stbi_image_free(pixels);

			VkCommandBufferBeginInfo commandBufferBeginInfo = Utility::Vulkan::CreateInfo::commandBufferBeginInfo();
			VK_VALIDATION(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

			VkImageMemoryBarrier imageMemoryBarrier{};
			imageMemoryBarrier.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
			imageMemoryBarrier.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.image = image.image;
			imageMemoryBarrier.subresourceRange.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
			imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
			imageMemoryBarrier.subresourceRange.levelCount = 1;
			imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
			imageMemoryBarrier.subresourceRange.layerCount = 1;
			imageMemoryBarrier.srcAccessMask = 0;
			imageMemoryBarrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
			vkCmdPipelineBarrier(commandBuffer,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
				0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

			VkBufferImageCopy bufferImageCopy = Utility::Vulkan::CreateInfo::bufferImageCopy(textureWidth, textureHeight);
			vkCmdCopyBufferToImage(commandBuffer, stagingBuffer.buffer, image.image, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);

			imageMemoryBarrier.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageMemoryBarrier.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageMemoryBarrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
			imageMemoryBarrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;
			vkCmdPipelineBarrier(commandBuffer,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

			VK_VALIDATION(vkEndCommandBuffer(commandBuffer));

			VkSubmitInfo submitInfo = Utility::Vulkan::CreateInfo::submitInfo();
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			queue->Submit(submitInfo, fence);
			VK_VALIDATION(vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX));

			// clean up
			vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
			vmaDestroyBuffer(allocator, stagingBuffer.buffer, stagingBuffer.allocation);
			vkDestroyFence(device, fence, pAllocationCallbacks);
		}
		return image;
	}

	inline MyosotisFW::VMAImage loadCubeImage(VkDevice device, MyosotisFW::System::Render::RenderQueue_ptr queue, VkCommandPool commandPool, VmaAllocator allocator, std::vector<std::string> fileNames, const VkAllocationCallbacks* pAllocationCallbacks = nullptr)
	{
		MyosotisFW::VMAImage image{};

		// loadCubeImage fence
		VkFence fence = VK_NULL_HANDLE;
		VkFenceCreateInfo fenceCreateInfo = Utility::Vulkan::CreateInfo::fenceCreateInfo();
		VK_VALIDATION(vkCreateFence(device, &fenceCreateInfo, pAllocationCallbacks, &fence));

		int textureWidth = -1;
		int textureHeight = -1;
		int textureChannels = -1;
		size_t imageSize = 0;
		std::vector<stbi_uc*> pixels{};
		for (uint32_t i = 0; i < fileNames.size(); i++)
		{
			std::filesystem::path absolutePath = std::filesystem::absolute(MyosotisFW::AppInfo::g_textureFolder + fileNames[i]);
			pixels.push_back(stbi_load(absolutePath.string().c_str(), &textureWidth, &textureHeight, &textureChannels, STBI_rgb_alpha));
			ASSERT(pixels[i], "Failed to load image: " + absolutePath.string());
			imageSize += textureWidth * textureHeight * textureChannels;
		}

		{// GPU image
			VkImageCreateInfo imageCreateInfo = Utility::Vulkan::CreateInfo::imageCreateInfo(VkFormat::VK_FORMAT_R8G8B8A8_SRGB, textureWidth, textureHeight);
			imageCreateInfo.arrayLayers = 6;
			imageCreateInfo.flags = VkImageCreateFlagBits::VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
			VmaAllocationCreateInfo allocationCreateInfo{};
			allocationCreateInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;
			VK_VALIDATION(vmaCreateImage(allocator, &imageCreateInfo, &allocationCreateInfo, &image.image, &image.allocation, &image.allocationInfo));
			VkImageViewCreateInfo imageViewCreateInfo = Utility::Vulkan::CreateInfo::imageViewCreateInfo(image.image, VkFormat::VK_FORMAT_R8G8B8A8_SRGB);
			imageViewCreateInfo.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_CUBE;
			imageViewCreateInfo.subresourceRange.layerCount = 6;
			VK_VALIDATION(vkCreateImageView(device, &imageViewCreateInfo, pAllocationCallbacks, &image.view));
		}

		MyosotisFW::Buffer stagingBuffer{};
		{// CPU buffer (staging buffer)
			VkBufferCreateInfo bufferCreateInfo = Utility::Vulkan::CreateInfo::bufferCreateInfo(imageSize, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
			VmaAllocationCreateInfo allocationCreateInfo{};
			allocationCreateInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY;
			VK_VALIDATION(vmaCreateBuffer(allocator, &bufferCreateInfo, &allocationCreateInfo, &stagingBuffer.buffer, &stagingBuffer.allocation, &stagingBuffer.allocationInfo));
		}

		{// CPU -> GPU
			VkCommandBuffer commandBuffer{};
			VkCommandBufferAllocateInfo commandBufferAllocateInfo = Utility::Vulkan::CreateInfo::commandBufferAllocateInfo(commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
			VK_VALIDATION(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer));

			void* data{};
			VK_VALIDATION(vmaMapMemory(allocator, stagingBuffer.allocation, &data));
			size_t offset = 0;
			for (size_t i = 0; i < pixels.size(); i++)
			{
				memcpy(static_cast<uint8_t*>(data) + offset, pixels[i], textureWidth * textureHeight * textureChannels);
				offset += textureWidth * textureHeight * textureChannels;
				stbi_image_free(pixels[i]);
			}
			vmaUnmapMemory(allocator, stagingBuffer.allocation);

			VkCommandBufferBeginInfo commandBufferBeginInfo = Utility::Vulkan::CreateInfo::commandBufferBeginInfo();
			VK_VALIDATION(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

			VkImageMemoryBarrier imageMemoryBarrier{};
			imageMemoryBarrier.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
			imageMemoryBarrier.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.image = image.image;
			imageMemoryBarrier.subresourceRange.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
			imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
			imageMemoryBarrier.subresourceRange.levelCount = 1;
			imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
			imageMemoryBarrier.subresourceRange.layerCount = 6;
			imageMemoryBarrier.srcAccessMask = 0;
			imageMemoryBarrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
			vkCmdPipelineBarrier(commandBuffer,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
				0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

			std::vector<VkBufferImageCopy> bufferImageCopy(pixels.size());
			for (size_t i = 0; i < pixels.size(); i++)
			{
				bufferImageCopy[i] = Utility::Vulkan::CreateInfo::bufferImageCopy(textureWidth, textureHeight);
				bufferImageCopy[i].bufferOffset = textureWidth * textureHeight * textureChannels * i;
				bufferImageCopy[i].imageSubresource.baseArrayLayer = static_cast<uint32_t>(i);
			}
			vkCmdCopyBufferToImage(commandBuffer, stagingBuffer.buffer, image.image, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(bufferImageCopy.size()), bufferImageCopy.data());

			imageMemoryBarrier.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageMemoryBarrier.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageMemoryBarrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
			imageMemoryBarrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

			VK_VALIDATION(vkEndCommandBuffer(commandBuffer));

			VkSubmitInfo submitInfo = Utility::Vulkan::CreateInfo::submitInfo();
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			queue->Submit(submitInfo, fence);
			VK_VALIDATION(vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX));

			// clean up
			vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
			vmaDestroyBuffer(allocator, stagingBuffer.buffer, stagingBuffer.allocation);
			vkDestroyFence(device, fence, pAllocationCallbacks);
		}
		return image;
	}
}
