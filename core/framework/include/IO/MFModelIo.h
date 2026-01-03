// Copyright (c) 2025 kong9812
#pragma once
#include <iostream>
#include <vector>

#include "AppInfo.h"
#include "Logger.h"
#include "Mesh.h"
#include "BasicMaterial.h"

namespace Utility::Loader
{
	inline std::pair<MyosotisFW::Mesh, MyosotisFW::BasicMaterial> DesterilizeMFModel(const char* path)
	{
		std::ifstream file{};
		file.open(path, std::ios::in | std::ios::binary);
		ASSERT(file.is_open(), "Failed to open mfmodel file: " + std::string(path));

		MyosotisFW::Mesh meshData{};
		MyosotisFW::BasicMaterial materialData{};

		// MeshInfo読み込み
		file.read(reinterpret_cast<char*>(&meshData.meshInfo), sizeof(MyosotisFW::MeshInfo));

		// 頂点データ読み込み
		meshData.vertex.resize(meshData.meshInfo.vertexFloatCount);
		file.read(reinterpret_cast<char*>(meshData.vertex.data()), sizeof(float) * meshData.meshInfo.vertexFloatCount);

		// Indexデータ読み込み
		meshData.index.resize(meshData.meshInfo.indexCount);
		file.read(reinterpret_cast<char*>(meshData.index.data()), sizeof(uint32_t) * meshData.meshInfo.indexCount);

		// Meshletデータ読み込み
		meshData.meshlet.resize(meshData.meshInfo.meshletCount);
		for (uint32_t i = 0; i < meshData.meshInfo.meshletCount; i++)
		{
			// MeshletInfo読み込み
			file.read(reinterpret_cast<char*>(&meshData.meshlet[i].meshletInfo), sizeof(MyosotisFW::MeshletInfo));
			// UniqueIndex読み込み
			meshData.meshlet[i].uniqueIndex.resize(meshData.meshlet[i].meshletInfo.vertexCount);
			file.read(reinterpret_cast<char*>(meshData.meshlet[i].uniqueIndex.data()), sizeof(uint32_t) * meshData.meshlet[i].meshletInfo.vertexCount);
			// Primitives読み込み
			meshData.meshlet[i].primitives.resize(static_cast<size_t>(meshData.meshlet[i].meshletInfo.primitiveCount) * 3);
			file.read(reinterpret_cast<char*>(meshData.meshlet[i].primitives.data()), sizeof(uint32_t) * meshData.meshlet[i].meshletInfo.primitiveCount * 3);
		}

		// MaterialInfo読み込み
		file.read(reinterpret_cast<char*>(&materialData.basicMaterialInfo), sizeof(MyosotisFW::BasicMaterialInfo));
		// BaseColorTexturePathの長さ 読み込み
		size_t length = 0;
		file.read(reinterpret_cast<char*>(&length), sizeof(uint32_t));
		// BaseColorTexturePath読み込み
		materialData.baseColorTexturePath.resize(length);
		file.read(reinterpret_cast<char*>(materialData.baseColorTexturePath.data()), sizeof(char) * length);
		// NormalTexturePathの長さ 読み込み
		length = 0;
		file.read(reinterpret_cast<char*>(&length), sizeof(uint32_t));
		// NormalTexturePath読み込み
		materialData.normalTexturePath.resize(length);
		file.read(reinterpret_cast<char*>(materialData.normalTexturePath.data()), sizeof(char) * length);

		file.close();
		return { meshData, materialData };
	}

	inline void SerializeMFModel(const char* path, const MyosotisFW::Mesh& meshes, const MyosotisFW::BasicMaterial& material)
	{
		std::ofstream file{};
		std::string fullPath = std::string(MyosotisFW::AppInfo::g_mfModelFolder) + path + ".mfmodel";
		file.open(fullPath, std::ios::out | std::ios::binary);
		ASSERT(file.is_open(), "Failed to open mfmodel file: " + fullPath);

		// MeshInfo書き込み
		file.write(reinterpret_cast<const char*>(&meshes.meshInfo), sizeof(MyosotisFW::MeshInfo));

		// 頂点データ書き込み
		file.write(reinterpret_cast<const char*>(meshes.vertex.data()), sizeof(float) * meshes.meshInfo.vertexFloatCount);

		// indexデータ書き込み
		file.write(reinterpret_cast<const char*>(meshes.index.data()), sizeof(uint32_t) * meshes.meshInfo.indexCount);

		// Meshletデータ書き込み
		for (const auto& meshlet : meshes.meshlet)
		{
			// MeshletInfo書き込み
			file.write(reinterpret_cast<const char*>(&meshlet.meshletInfo), sizeof(MyosotisFW::MeshletInfo));
			// UniqueIndex書き込み
			file.write(reinterpret_cast<const char*>(meshlet.uniqueIndex.data()), sizeof(uint32_t) * meshlet.meshletInfo.vertexCount);
			// Primitives書き込み
			file.write(reinterpret_cast<const char*>(meshlet.primitives.data()), sizeof(uint32_t) * meshlet.meshletInfo.primitiveCount * 3);
		}

		// MaterialInfo書き込み
		file.write(reinterpret_cast<const char*>(&material.basicMaterialInfo), sizeof(MyosotisFW::BasicMaterialInfo));
		// BaseColorTexturePathの長さ 書き込み
		size_t length = material.baseColorTexturePath.length();
		file.write(reinterpret_cast<const char*>(&length), sizeof(uint32_t));
		// BaseColorTexturePath書き込み
		file.write(reinterpret_cast<const char*>(material.baseColorTexturePath.data()), sizeof(char) * length);
		// NormalTexturePathの長さ 書き込み
		length = material.normalTexturePath.length();
		file.write(reinterpret_cast<const char*>(&length), sizeof(uint32_t));
		// NormalTexturePath書き込み
		file.write(reinterpret_cast<const char*>(material.normalTexturePath.data()), sizeof(char) * length);

		file.close();
	}

	inline std::vector<std::pair<MyosotisFW::Mesh, MyosotisFW::BasicMaterial>> loadMFModel(std::string fileName)
	{
#ifdef DEBUG
		Logger::Debug("[VK_Loader] Start load: " + fileName);
		auto start = std::chrono::high_resolution_clock::now();
#endif
		std::string fullPath = std::string(MyosotisFW::AppInfo::g_mfModelFolder) + fileName;
		std::vector<std::pair<MyosotisFW::Mesh, MyosotisFW::BasicMaterial>> meshes{};

		meshes.push_back(DesterilizeMFModel(fullPath.c_str()));
#ifdef DEBUG
		Logger::Debug("[VK_Loader] End load: " + fileName +
			"(" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count()) + "ms)");
#endif
		return meshes;
	}
}