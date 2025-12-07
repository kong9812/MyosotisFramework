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

#include "imeshoptimizer.h"
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

	inline std::vector<MyosotisFW::Mesh> loadTerrainMesh(std::string fileName)
	{
		std::filesystem::path absolutePath = std::filesystem::absolute(MyosotisFW::AppInfo::g_terrainFolder + fileName);

		int32_t textureWidth = -1;
		int32_t textureHeight = -1;
		int32_t textureChannels = -1;
		stbi_uc* pixels = stbi_load(absolutePath.string().c_str(), &textureWidth, &textureHeight, &textureChannels, STBI_grey);
		ASSERT(pixels, "Failed to load image: " + absolutePath.string());
		size_t imageSize = textureWidth * textureHeight * textureChannels;

		// チャンクサイズ (MyosotisFW::AppInfo::g_terrainChunkSize)
		const glm::uvec2 chunkSize = glm::uvec2(1280, 1280);
		const glm::uvec2 chunkCount = glm::uvec2(
			(textureWidth + chunkSize.x - 1) / chunkSize.x,
			(textureHeight + chunkSize.y - 1) / chunkSize.y
		);
		// サンプリング間隔
		const uint32_t samplingStep = 10;

		std::vector<MyosotisFW::Mesh> meshes;
		meshes.resize(chunkCount.x * chunkCount.y);

		// 全チャンクループ
		for (uint32_t cy = 0; cy < chunkCount.y; cy++)
		{
			for (uint32_t cx = 0; cx < chunkCount.x; cx++)
			{
				const uint32_t chunkIndex = cy * chunkCount.x + cx;
				MyosotisFW::Mesh& mesh = meshes[chunkIndex];
				mesh.meshInfo.AABBMin = glm::vec4(FLT_MAX);
				mesh.meshInfo.AABBMax = glm::vec4(-FLT_MAX);

				// AABB用
				std::vector<glm::vec3> tmpPositions{};
				glm::vec3 meshAABBMin(FLT_MAX);
				glm::vec3 meshAABBMax(-FLT_MAX);

				// チャンクの元画像領域（start/end）
				uint32_t startX = cx * chunkSize.x;
				uint32_t startY = cy * chunkSize.y;
				uint32_t endX = std::min(startX + chunkSize.x, static_cast<uint32_t>(textureWidth));
				uint32_t endY = std::min(startY + chunkSize.y, static_cast<uint32_t>(textureHeight));

				// vertex
				for (uint32_t y = startY; y < endY; y += samplingStep)
				{
					for (uint32_t x = startX; x < endX; x += samplingStep)
					{
						// 8bit画像(0-255) 高さ値を0～1に変換
						float heightValue = (float)pixels[y * textureWidth + x] / 255.0f;

						{// Position
							mesh.vertex.insert(mesh.vertex.end(),
								{
									static_cast<float>(x) * MyosotisFW::AppInfo::g_terrainScale.x,
									heightValue * MyosotisFW::AppInfo::g_terrainScale.y,
									static_cast<float>(y) * MyosotisFW::AppInfo::g_terrainScale.z,
									1.0f
								});
							// AABB用Position保存
							tmpPositions.push_back(glm::vec3(
								static_cast<float>(x) * MyosotisFW::AppInfo::g_terrainScale.x,
								heightValue * MyosotisFW::AppInfo::g_terrainScale.y,
								static_cast<float>(y) * MyosotisFW::AppInfo::g_terrainScale.z)
							);
						}

						{// Normal (仮)
							mesh.vertex.insert(mesh.vertex.end(),
								{
									0.0f,
									1.0f,
									0.0f
								});
						}
						{// UV
							mesh.vertex.insert(mesh.vertex.end(),
								{
									static_cast<float>(x) / static_cast<float>(textureWidth),
									static_cast<float>(y) / static_cast<float>(textureHeight)
								});
						}
						{// Color
							mesh.vertex.insert(mesh.vertex.end(),
								{
									1.0f,
									1.0f,
									1.0f,
									1.0f
								});
						}
					}
				}

				// index (samplingStep分だけ縮んだグリッドの横幅/高さ)
				std::vector<uint32_t> index;
				uint32_t localW = (endX - startX) / samplingStep;
				uint32_t localH = (endY - startY) / samplingStep;
				for (uint32_t iy = 0; iy < localH - 1; iy++)
				{
					for (uint32_t ix = 0; ix < localW - 1; ix++)
					{
						uint32_t i0 = iy * localW + ix;
						uint32_t i1 = i0 + 1;
						uint32_t i2 = i0 + localW;
						uint32_t i3 = i2 + 1;

						// 三角形 1
						index.push_back(i0);
						index.push_back(i2);
						index.push_back(i1);

						// 三角形 2
						index.push_back(i1);
						index.push_back(i2);
						index.push_back(i3);
					}
				}

				// Meshlet生成
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
					&(mesh.vertex[0]),
					mesh.vertex.size() / (4 + 3 + 2 + 4),
					sizeof(float) * (4 + 3 + 2 + 4),	// Position(4) + Normal(3) + UV(2) + Color(4)
					MyosotisFW::AppInfo::g_maxMeshletVertices,
					MyosotisFW::AppInfo::g_maxMeshletPrimitives,
					0.0f);

				mesh.meshlet.reserve(meshletCount);
				for (size_t i = 0; i < meshletCount; i++)
				{
					const meshopt_Meshlet& src = meshlets[i];
					MyosotisFW::Meshlet dst{};

					// UniqueIndex (GlobalIndex)
					dst.uniqueIndex.reserve(src.vertex_count);
					for (uint32_t v = 0; v < src.vertex_count; v++)
					{
						uint32_t globalIndex = meshletVertices[i * MyosotisFW::AppInfo::g_maxMeshletVertices + v];
						dst.uniqueIndex.push_back(globalIndex);
					}

					// Primitives (LocalIndex)
					dst.primitives.reserve(src.triangle_count * 3);
					for (uint32_t t = 0; t < src.triangle_count; t++)
					{
						uint8_t i0 = meshletTriangles[i * MyosotisFW::AppInfo::g_maxMeshletPrimitives * 3 + t * 3 + 0];
						uint8_t i1 = meshletTriangles[i * MyosotisFW::AppInfo::g_maxMeshletPrimitives * 3 + t * 3 + 1];
						uint8_t i2 = meshletTriangles[i * MyosotisFW::AppInfo::g_maxMeshletPrimitives * 3 + t * 3 + 2];
						dst.primitives.push_back(i0);
						dst.primitives.push_back(i1);
						dst.primitives.push_back(i2);
					}

					// AABB
					glm::vec3 p0 = tmpPositions[meshletVertices[src.vertex_offset + 0]];
					dst.meshletInfo.AABBMin = glm::vec4(p0, 0.0f);
					dst.meshletInfo.AABBMax = glm::vec4(p0, 0.0f);
					meshAABBMin = glm::min(meshAABBMin, p0);
					meshAABBMax = glm::max(meshAABBMax, p0);
					for (size_t v = 1; v < src.vertex_count; v++)
					{
						uint32_t vertexIndex = meshletVertices[src.vertex_offset + v];
						const glm::vec3& pos = tmpPositions[vertexIndex];
						dst.meshletInfo.AABBMin = glm::min(dst.meshletInfo.AABBMin, glm::vec4(pos, 0.0f));
						dst.meshletInfo.AABBMax = glm::max(dst.meshletInfo.AABBMax, glm::vec4(pos, 0.0f));
						meshAABBMin = glm::min(meshAABBMin, pos);
						meshAABBMax = glm::max(meshAABBMax, pos);
					}

					dst.meshletInfo.vertexCount = src.vertex_count;
					dst.meshletInfo.primitiveCount = src.triangle_count;

					mesh.meshlet.push_back(dst);
				}

				// MeshInfo/AABB は後で算出
				mesh.meshInfo.AABBMin = glm::vec4(meshAABBMin, 0.0f);
				mesh.meshInfo.AABBMax = glm::vec4(meshAABBMax, 0.0f);
				mesh.meshInfo.meshletCount = meshletCount;
				mesh.meshInfo.vertexFloatCount = static_cast<uint32_t>(mesh.vertex.size());
			}
		}

		stbi_image_free(pixels);

		return meshes;
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
