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
	// 共通の定数
	constexpr uint32_t POSITION_COMPONENTS = 4; // x,y,z,w
	constexpr uint32_t NORMAL_COMPONENTS = 3;
	constexpr uint32_t UV_COMPONENTS = 2;
	constexpr uint32_t COLOR_COMPONENTS = 4;
	constexpr uint32_t FLOATS_PER_VERTEX = POSITION_COMPONENTS + NORMAL_COMPONENTS + UV_COMPONENTS + COLOR_COMPONENTS; // 13
	constexpr uint32_t NORMAL_OFFSET = POSITION_COMPONENTS; // 4

	// 8bit高さマップを読み込む
	inline stbi_uc* LoadGrayImage(const std::filesystem::path& path, int32_t& outWidth, int32_t& outHeight, int32_t& outChannels)
	{
		stbi_uc* pixels = stbi_load(path.string().c_str(), &outWidth, &outHeight, &outChannels, STBI_grey);
		ASSERT(pixels, std::string("Failed to load image: ") + path.string());
		return pixels;
	}

	// グリッド(幅x高さ)から法線を計算する
	inline void ComputeNormalsFromGrid(const std::vector<glm::vec3>& positions, std::vector<glm::vec3>& outNormals, uint32_t width, uint32_t height)
	{
		outNormals.clear();
		outNormals.resize(positions.size(), glm::vec3(0.0f));

		if (width < 2 || height < 2)
			return;

		for (uint32_t y = 0; y < height - 1; y++)
		{
			for (uint32_t x = 0; x < width - 1; x++)
			{
				const uint32_t topLeft = y * width + x;
				const uint32_t topRight = topLeft + 1;
				const uint32_t bottomLeft = (y + 1) * width + x;
				const uint32_t bottomRight = bottomLeft + 1;

				// 三角形 1
				const glm::vec3 v0 = positions[topLeft];
				const glm::vec3 v1 = positions[bottomLeft];
				const glm::vec3 v2 = positions[topRight];
				const glm::vec3 normal1 = glm::normalize(glm::cross(v1 - v0, v2 - v0));

				// 三角形 2
				const glm::vec3 u0 = positions[topRight];
				const glm::vec3 u1 = positions[bottomLeft];
				const glm::vec3 u2 = positions[bottomRight];
				const glm::vec3 normal2 = glm::normalize(glm::cross(u1 - u0, u2 - u0));

				outNormals[topLeft] += normal1;
				outNormals[bottomLeft] += (normal1 + normal2);
				outNormals[topRight] += (normal1 + normal2);
				outNormals[bottomRight] += normal2;
			}
		}

		for (auto& n : outNormals)
		{
			n = glm::normalize(n);
		}
	}

	inline void CalculateTerrainNormals(MyosotisFW::RawMeshData& mesh, uint32_t width, uint32_t height)
	{
		std::vector<glm::vec3> positions{};
		positions.reserve(mesh.position.size());
		for (const auto& p : mesh.position)
			positions.push_back(glm::vec3(p));

		std::vector<glm::vec3> normals{};
		ComputeNormalsFromGrid(positions, normals, width, height);

		mesh.normal = std::move(normals);
	}

	inline void CalculateTerrainNormals2(const std::vector<glm::vec3>& position, std::vector<glm::vec3>& normal, uint32_t width, uint32_t height)
	{
		ComputeNormalsFromGrid(position, normal, width, height);
	}

	inline uint32_t GetNearestDivisor(uint32_t step, uint32_t target)
	{
		uint32_t best = 1;
		uint32_t bestDiff = UINT32_MAX;

		for (uint32_t d = 1; d <= target; d++)
		{
			if (target % d != 0) continue;

			uint32_t diff = (step > d) ? (step - d) : (d - step);
			if (diff < bestDiff)
			{
				bestDiff = diff;
				best = d;
			}
		}
		return best;
	}

	inline std::vector<MyosotisFW::Mesh> loadTerrainMesh(std::string fileName)
	{
		const std::filesystem::path absolutePath = std::filesystem::absolute(MyosotisFW::AppInfo::g_terrainFolder + fileName);

		int32_t textureWidth = -1;
		int32_t textureHeight = -1;
		int32_t textureChannels = -1;
		stbi_uc* pixels = LoadGrayImage(absolutePath, textureWidth, textureHeight, textureChannels);
		size_t imageSize = static_cast<size_t>(textureWidth) * static_cast<size_t>(textureHeight) * static_cast<size_t>(textureChannels);

		// チャンク設定
		const glm::uvec2 chunkSize = glm::uvec2(1280, 1280);
		const glm::uvec2 chunkCount = glm::uvec2(
			(static_cast<uint32_t>(textureWidth) + chunkSize.x - 1) / chunkSize.x,
			(static_cast<uint32_t>(textureHeight) + chunkSize.y - 1) / chunkSize.y
		);

		uint32_t samplingStep = 10;
		samplingStep = GetNearestDivisor(samplingStep, chunkSize.x);

		std::vector<MyosotisFW::Mesh> meshes{};
		meshes.resize(chunkCount.x * chunkCount.y);

		for (uint32_t cy = 0; cy < chunkCount.y; cy++)
		{
			for (uint32_t cx = 0; cx < chunkCount.x; cx++)
			{
				const uint32_t chunkIndex = cy * chunkCount.x + cx;
				MyosotisFW::Mesh& mesh = meshes[chunkIndex];
				mesh.meshInfo.AABBMin = glm::vec4(FLT_MAX);
				mesh.meshInfo.AABBMax = glm::vec4(-FLT_MAX);

				std::vector<glm::vec3> tmpPositions{};
				tmpPositions.reserve((chunkSize.x / samplingStep) * (chunkSize.y / samplingStep));
				glm::vec3 meshAABBMin(FLT_MAX);
				glm::vec3 meshAABBMax(-FLT_MAX);

				const uint32_t startX = cx * chunkSize.x;
				const uint32_t startY = cy * chunkSize.y;
				const uint32_t endX = std::min(startX + chunkSize.x, static_cast<uint32_t>(textureWidth));
				const uint32_t endY = std::min(startY + chunkSize.y, static_cast<uint32_t>(textureHeight));

				// 頂点生成
				for (uint32_t y = startY; y <= endY; y += samplingStep)
				{
					for (uint32_t x = startX; x <= endX; x += samplingStep)
					{
						const float heightValue = static_cast<float>(pixels[y * textureWidth + x]) / 255.0f;

						// Position
						mesh.vertex.insert(mesh.vertex.end(), {
								static_cast<float>(x) * MyosotisFW::AppInfo::g_terrainScale.x,
								heightValue * MyosotisFW::AppInfo::g_terrainScale.y,
								static_cast<float>(y) * MyosotisFW::AppInfo::g_terrainScale.z,
								1.0f
							});
						// tmpPositions(AABB/meshlet用)
						tmpPositions.emplace_back(
							static_cast<float>(x) * MyosotisFW::AppInfo::g_terrainScale.x,
							heightValue * MyosotisFW::AppInfo::g_terrainScale.y,
							static_cast<float>(y) * MyosotisFW::AppInfo::g_terrainScale.z
						);

						// Normal(仮)
						mesh.vertex.insert(mesh.vertex.end(), { 0.0f, 1.0f, 0.0f });

						// UV
						mesh.vertex.insert(mesh.vertex.end(), {
							static_cast<float>(x) / static_cast<float>(textureWidth),
							static_cast<float>(y) / static_cast<float>(textureHeight)
							});

						// Color
						mesh.vertex.insert(mesh.vertex.end(), { 1.0f, 1.0f, 1.0f, 1.0f });
					}
				}

				// ローカル幅/高さ
				const uint32_t localW = ((endX - startX) / samplingStep) + 1;
				const uint32_t localH = ((endY - startY) / samplingStep) + 1;

				// normal計算
				std::vector<glm::vec3> tmpNormal{};
				ComputeNormalsFromGrid(tmpPositions, tmpNormal, localW, localH);
				for (uint32_t i = 0; i < tmpNormal.size(); i++)
				{
					uint32_t base = i * FLOATS_PER_VERTEX + NORMAL_OFFSET;
					mesh.vertex[base + 0] = tmpNormal[i].x;
					mesh.vertex[base + 1] = tmpNormal[i].y;
					mesh.vertex[base + 2] = tmpNormal[i].z;
				}

				// index作成
				std::vector<uint32_t> index{};
				index.reserve((localW - 1) * (localH - 1) * 6);
				for (uint32_t iy = 0; iy < localH - 1; iy++)
				{
					for (uint32_t ix = 0; ix < localW - 1; ix++)
					{
						const uint32_t i0 = iy * localW + ix;
						const uint32_t i1 = i0 + 1;
						const uint32_t i2 = i0 + localW;
						const uint32_t i3 = i2 + 1;

						index.push_back(i0);
						index.push_back(i2);
						index.push_back(i1);

						index.push_back(i1);
						index.push_back(i2);
						index.push_back(i3);
					}
				}

				// Meshlet生成
				size_t maxMeshlets = meshopt_buildMeshletsBound(index.size(), MyosotisFW::AppInfo::g_maxMeshletVertices, MyosotisFW::AppInfo::g_maxMeshletPrimitives);

				std::vector<meshopt_Meshlet> meshlets(maxMeshlets);
				std::vector<uint32_t> meshletVertices(maxMeshlets * MyosotisFW::AppInfo::g_maxMeshletVertices);
				std::vector<uint8_t> meshletTriangles(maxMeshlets * MyosotisFW::AppInfo::g_maxMeshletPrimitives * 3);

				size_t meshletCount = meshopt_buildMeshlets(
					meshlets.data(),
					meshletVertices.data(),
					meshletTriangles.data(),
					index.data(),
					index.size(),
					&(mesh.vertex[0]),
					mesh.vertex.size() / FLOATS_PER_VERTEX,
					size_t(sizeof(float) * FLOATS_PER_VERTEX),
					MyosotisFW::AppInfo::g_maxMeshletVertices,
					MyosotisFW::AppInfo::g_maxMeshletPrimitives,
					0.0f);

				mesh.meshlet.reserve(meshletCount);
				for (size_t i = 0; i < meshletCount; i++)
				{
					const meshopt_Meshlet& src = meshlets[i];
					MyosotisFW::Meshlet dst{};

					dst.uniqueIndex.reserve(src.vertex_count);
					for (size_t v = 0; v < src.vertex_count; v++)
					{
						uint32_t vertexIndex = meshletVertices[src.vertex_offset + v];
						dst.uniqueIndex.push_back(vertexIndex);
					}

					dst.primitives.reserve(src.triangle_count * 3);
					for (size_t t = 0; t < src.triangle_count * 3; t++)
					{
						uint32_t triangleIndex = static_cast<uint32_t>(meshletTriangles[src.triangle_offset + t]);
						dst.primitives.push_back(triangleIndex);
					}

					// AABB 計算
					auto p0 = tmpPositions[meshletVertices[src.vertex_offset + 0]];
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

				// MeshInfo/AABB 設定
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
		const std::filesystem::path absolutePath = std::filesystem::absolute(MyosotisFW::AppInfo::g_terrainFolder + fileName);

		int32_t textureWidth = -1;
		int32_t textureHeight = -1;
		int32_t textureChannels = -1;
		stbi_uc* pixels = LoadGrayImage(absolutePath, textureWidth, textureHeight, textureChannels);

		MyosotisFW::RawMeshData rawMeshData{};

		// 元実装と同じ samplingStep
		const uint32_t samplingStep = 60;

		const uint32_t sampledWidth = static_cast<uint32_t>(textureWidth) / samplingStep;
		const uint32_t sampledHeight = static_cast<uint32_t>(textureHeight) / samplingStep;
		size_t totalVertices = static_cast<size_t>(sampledWidth) * static_cast<size_t>(sampledHeight);

		rawMeshData.position.reserve(totalVertices);
		rawMeshData.normal.reserve(totalVertices);
		rawMeshData.uv.reserve(totalVertices);
		rawMeshData.color.reserve(totalVertices);
		rawMeshData.vertex.reserve(totalVertices * FLOATS_PER_VERTEX);

		for (uint32_t y = 0; y < textureHeight; y += samplingStep)
		{
			for (uint32_t x = 0; x < textureWidth; x += samplingStep)
			{
				const float heightValue = static_cast<float>(pixels[y * textureWidth + x]) / 255.0f;

				// Position
				glm::vec4 p = glm::vec4(
					static_cast<float>(x) * MyosotisFW::AppInfo::g_terrainScale.x,
					static_cast<float>(y) * MyosotisFW::AppInfo::g_terrainScale.z,
					heightValue * MyosotisFW::AppInfo::g_terrainScale.y,
					1.0f
				);
				rawMeshData.position.push_back(p);
				rawMeshData.vertex.insert(rawMeshData.vertex.end(), { p.x, p.y, p.z, p.w });

				// Normal (固定: 上向き)
				glm::vec3 n = glm::vec3(0.0f, 1.0f, 0.0f);
				rawMeshData.normal.push_back(n);
				rawMeshData.vertex.insert(rawMeshData.vertex.end(), { n.x, n.y, n.z });

				// UV
				glm::vec2 u = glm::vec2(
					static_cast<float>(x) / (textureWidth - 1),
					static_cast<float>(y) / (textureHeight - 1)
				);
				rawMeshData.uv.push_back(u);
				rawMeshData.vertex.insert(rawMeshData.vertex.end(), { u.x, u.y });

				// Color (固定: 白)
				glm::vec4 c = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				rawMeshData.color.push_back(c);
				rawMeshData.vertex.insert(rawMeshData.vertex.end(), { c.r, c.g, c.b, c.a });
			}
		}

		// Index 生成
		for (uint32_t y = 0; y < sampledHeight - 1; y++)
		{
			for (uint32_t x = 0; x < sampledWidth - 1; x++)
			{
				const uint32_t topLeft = y * sampledWidth + x;
				const uint32_t topRight = topLeft + 1;
				const uint32_t bottomLeft = (y + 1) * sampledWidth + x;
				const uint32_t bottomRight = bottomLeft + 1;

				rawMeshData.index.push_back(topLeft);
				rawMeshData.index.push_back(bottomLeft);
				rawMeshData.index.push_back(topRight);

				rawMeshData.index.push_back(topRight);
				rawMeshData.index.push_back(bottomLeft);
				rawMeshData.index.push_back(bottomRight);
			}
		}

		stbi_image_free(pixels);

		// 法線計算
		CalculateTerrainNormals(rawMeshData, sampledWidth, sampledHeight);

		// vertex 配列のノーマルを更新（元実装と同じオフセット）
		for (size_t i = 0; i < rawMeshData.normal.size(); i++)
		{
			auto base = i * FLOATS_PER_VERTEX + NORMAL_OFFSET;
			rawMeshData.vertex[base + 0] = rawMeshData.normal[i].x;
			rawMeshData.vertex[base + 1] = rawMeshData.normal[i].y;
			rawMeshData.vertex[base + 2] = rawMeshData.normal[i].z;
		}
		return rawMeshData;
	}
}