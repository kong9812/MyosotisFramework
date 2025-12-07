// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include <fstream>

#include "istb_image.h"
#include "AppInfo.h"

#include "Logger.h"
#include "Image.h"
#include "Buffer.h"
#include "VK_CreateInfo.h"
#include "VK_Validation.h"
#include "RenderQueue.h"
#include "RawMeshData.h"

#include "Mesh.h"

namespace Utility::Loader {
	inline void CalculateTerrainNormals(MyosotisFW::RawMeshData& mesh, uint32_t width, uint32_t height)
	{
		mesh.normal.resize(mesh.position.size(), glm::vec3(0.0f));

		for (uint32_t y = 0; y < height - 1; y++)
		{
			for (uint32_t x = 0; x < width - 1; x++)
			{
				// 頂点インデックス
				uint32_t topLeft = y * width + x;
				uint32_t topRight = topLeft + 1;
				uint32_t bottomLeft = (y + 1) * width + x;
				uint32_t bottomRight = bottomLeft + 1;

				// 三角形 1
				glm::vec3 v0 = glm::vec3(mesh.position[topLeft]);
				glm::vec3 v1 = glm::vec3(mesh.position[bottomLeft]);
				glm::vec3 v2 = glm::vec3(mesh.position[topRight]);
				glm::vec3 normal1 = glm::normalize(glm::cross(v1 - v0, v2 - v0));

				// 三角形 2
				v0 = glm::vec3(mesh.position[topRight]);
				v1 = glm::vec3(mesh.position[bottomLeft]);
				v2 = glm::vec3(mesh.position[bottomRight]);
				glm::vec3 normal2 = glm::normalize(glm::cross(v1 - v0, v2 - v0));

				// 頂点に加算（平均化）
				mesh.normal[topLeft] += normal1;
				mesh.normal[bottomLeft] += normal1 + normal2;
				mesh.normal[topRight] += normal1 + normal2;
				mesh.normal[bottomRight] += normal2;
			}
		}

		// 正規化
		for (auto& n : mesh.normal)
		{
			n = glm::normalize(n);
		}
	}

	inline MyosotisFW::RawMeshData loadTerrain(std::string fileName)
	{
		std::filesystem::path absolutePath = std::filesystem::absolute(MyosotisFW::AppInfo::g_terrainFolder + fileName);

		int32_t textureWidth = -1;
		int32_t textureHeight = -1;
		int32_t textureChannels = -1;
		stbi_uc* pixels = stbi_load(absolutePath.string().c_str(), &textureWidth, &textureHeight, &textureChannels, STBI_grey);
		ASSERT(pixels, "Failed to load image: " + absolutePath.string());
		size_t imageSize = textureWidth * textureHeight * textureChannels;

		// メッシュ生成
		MyosotisFW::RawMeshData rawMeshData{};

		// --- 【サンプリングとパフォーマンス設定】 ---
		// ハイトマップから頂点データを抽出する間隔 (サンプリング数決定)
		// 値が大きいほど、生成される頂点数が減り、メッシュが粗くなる (例: 4 で 1/16 の頂点数)
		const uint32_t samplingStep = 60;

		uint32_t sampledWidth = textureWidth / samplingStep;
		uint32_t sampledHeight = textureHeight / samplingStep;
		size_t totalVertices = sampledWidth * sampledHeight;

		rawMeshData.position.reserve(totalVertices);
		rawMeshData.normal.reserve(totalVertices);
		rawMeshData.uv.reserve(totalVertices);
		rawMeshData.color.reserve(totalVertices);
		rawMeshData.vertex.reserve(totalVertices * (4 + 3 + 2 + 4)); // Position(4) + Normal(3) + UV(2) + Color(4)

		// Vertex
		// samplingStepに従い、テクスチャを飛び飛びにサンプリング
		for (uint32_t y = 0; y < textureHeight; y += samplingStep)
		{
			for (uint32_t x = 0; x < textureWidth; x += samplingStep)
			{
				// 8bit画像(0-255) 高さ値を0～1に変換
				float heightValue = (float)pixels[y * textureWidth + x] / 255.0f;

				{// Position
					glm::vec4 p{};
					p.x = (float)x * MyosotisFW::AppInfo::g_terrainScale.x;
					p.z = (float)y * MyosotisFW::AppInfo::g_terrainScale.z;
					p.y = heightValue * MyosotisFW::AppInfo::g_terrainScale.y;
					p.w = 1.0f;
					rawMeshData.position.push_back(p);
					rawMeshData.vertex.insert(rawMeshData.vertex.end(), { p.x, p.y, p.z, p.w });
				}
				{// Normal (固定: 上向き)
					glm::vec3 n{};
					n.x = 0.0f;
					n.y = 1.0f;
					n.z = 0.0f;
					rawMeshData.normal.push_back(n);
					rawMeshData.vertex.insert(rawMeshData.vertex.end(), { n.x, n.y, n.z });
				}
				{// UV
					glm::vec2 u{};
					u.x = (float)x / (textureWidth - 1);
					u.y = (float)y / (textureHeight - 1);
					rawMeshData.uv.push_back(u);
					rawMeshData.vertex.insert(rawMeshData.vertex.end(), { u.x, u.y });
				}
				{// Color (固定: 白)
					glm::vec4 c{};
					c.r = 1.0f;
					c.g = 1.0f;
					c.b = 1.0f;
					c.a = 1.0f;
					rawMeshData.color.push_back(c);
					rawMeshData.vertex.insert(rawMeshData.vertex.end(), { c.r, c.g, c.b, c.a });
				}
			}
		}

		// Index
		// サンプリング後の頂点数に基づきインデックスを生成
		for (uint32_t y = 0; y < sampledHeight - 1; y++)
		{
			for (uint32_t x = 0; x < sampledWidth - 1; x++)
			{
				// 頂点IDはRawMeshDataに追加された順序 (0からカウントアップ)
				uint32_t topLeft = y * sampledWidth + x;
				uint32_t topRight = topLeft + 1;
				uint32_t bottomLeft = (y + 1) * sampledWidth + x;
				uint32_t bottomRight = bottomLeft + 1;

				// Tri 1
				rawMeshData.index.push_back(topLeft);
				rawMeshData.index.push_back(bottomLeft);
				rawMeshData.index.push_back(topRight);

				// Tri 2
				rawMeshData.index.push_back(topRight);
				rawMeshData.index.push_back(bottomLeft);
				rawMeshData.index.push_back(bottomRight);
			}
		}

		// 用済み画像の破棄
		stbi_image_free(pixels);

		// 法線計算
		CalculateTerrainNormals(rawMeshData, sampledWidth, sampledHeight);
		// vertex 配列のノーマルを更新
		{
			constexpr uint32_t FLOATS_PER_VERTEX = 13;
			constexpr uint32_t NORMAL_OFFSET = 4;

			for (size_t i = 0; i < rawMeshData.normal.size(); i++)
			{
				size_t base = i * FLOATS_PER_VERTEX + NORMAL_OFFSET;

				rawMeshData.vertex[base + 0] = rawMeshData.normal[i].x;
				rawMeshData.vertex[base + 1] = rawMeshData.normal[i].y;
				rawMeshData.vertex[base + 2] = rawMeshData.normal[i].z;
			}
		}

		return rawMeshData;
	}
}
