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
	// 8bit高さマップを読み込む
	inline stbi_uc* LoadGrayImage(const std::filesystem::path& path, int32_t& outWidth, int32_t& outHeight, int32_t& outChannels)
	{
		stbi_uc* pixels = stbi_load(path.string().c_str(), &outWidth, &outHeight, &outChannels, STBI_grey);
		ASSERT(pixels, std::string("Failed to load image: ") + path.string());
		return pixels;
	}

	// グリッド(幅x高さ)から法線を計算する
	inline void ComputeNormalsFromGrid(std::vector<MyosotisFW::VertexData>& vertex, uint32_t width, uint32_t height)
	{
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
				const glm::vec3 v0 = vertex[topLeft].position;
				const glm::vec3 v1 = vertex[bottomLeft].position;
				const glm::vec3 v2 = vertex[topRight].position;
				const glm::vec3 normal1 = glm::normalize(glm::cross(v1 - v0, v2 - v0));

				// 三角形 2
				const glm::vec3 u0 = vertex[topRight].position;
				const glm::vec3 u1 = vertex[bottomLeft].position;
				const glm::vec3 u2 = vertex[bottomRight].position;
				const glm::vec3 normal2 = glm::normalize(glm::cross(u1 - u0, u2 - u0));

				vertex[topLeft].normal += normal1;
				vertex[bottomLeft].normal += (normal1 + normal2);
				vertex[topRight].normal += (normal1 + normal2);
				vertex[bottomRight].normal += normal2;
			}
		}

		for (auto& n : vertex)
		{
			n.normal = glm::normalize(n.normal);
		}
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

	inline std::vector<std::pair<MyosotisFW::Mesh, MyosotisFW::BasicMaterial>> loadTerrainMesh(std::string fileName)
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

		uint32_t samplingStep = 50;
		samplingStep = GetNearestDivisor(samplingStep, chunkSize.x);

		std::vector<std::pair<MyosotisFW::Mesh, MyosotisFW::BasicMaterial>> meshes{};
		meshes.resize(static_cast<size_t>(chunkCount.x) * static_cast<size_t>(chunkCount.y));

		for (uint32_t cy = 0; cy < chunkCount.y; cy++)
		{
			for (uint32_t cx = 0; cx < chunkCount.x; cx++)
			{
				const uint32_t chunkIndex = cy * chunkCount.x + cx;
				MyosotisFW::Mesh& mesh = meshes[chunkIndex].first;
				mesh.meshInfo.AABBMin = glm::vec4(FLT_MAX);
				mesh.meshInfo.AABBMax = glm::vec4(-FLT_MAX);

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
						glm::vec3 v = glm::vec3(
							static_cast<float>(x) * MyosotisFW::AppInfo::g_terrainScale.x,
							heightValue * MyosotisFW::AppInfo::g_terrainScale.y,
							static_cast<float>(y) * MyosotisFW::AppInfo::g_terrainScale.z
						);

						// Normal(仮)
						glm::vec3 n = glm::vec3(0.0f, 1.0f, 0.0f);

						// UV0
						glm::vec2 u0 = glm::vec2(
							static_cast<float>(x) / static_cast<float>(textureWidth),
							static_cast<float>(y) / static_cast<float>(textureHeight)
						);
						// UV1(仮)
						glm::vec2 u1 = glm::vec2(0.0f);

						// Color
						glm::vec4 c = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

						mesh.vertex.push_back({ v, n, u0, u1, c });
					}
				}

				// ローカル幅/高さ
				const uint32_t localW = ((endX - startX) / samplingStep) + 1;
				const uint32_t localH = ((endY - startY) / samplingStep) + 1;

				// normal計算
				ComputeNormalsFromGrid(mesh.vertex, localW, localH);

				// index作成
				mesh.index.reserve(static_cast<size_t>((localW - 1) * (localH - 1) * 6));
				for (uint32_t iy = 0; iy < localH - 1; iy++)
				{
					for (uint32_t ix = 0; ix < localW - 1; ix++)
					{
						const uint32_t i0 = iy * localW + ix;
						const uint32_t i1 = i0 + 1;
						const uint32_t i2 = i0 + localW;
						const uint32_t i3 = i2 + 1;

						mesh.index.push_back(i0);
						mesh.index.push_back(i2);
						mesh.index.push_back(i1);

						mesh.index.push_back(i1);
						mesh.index.push_back(i2);
						mesh.index.push_back(i3);
					}
				}

				// UV1
				mesh.meshInfo.atlasSize = xatlas::BuildLightmapUV(mesh.vertex, mesh.index);

				// Meshlet
				meshoptimizer::BuildMeshletData(mesh, mesh.index,
					MyosotisFW::AppInfo::g_maxMeshletVertices,
					MyosotisFW::AppInfo::g_maxMeshletPrimitives);

				// AABB
				mesh.meshInfo.AABBMin = glm::vec4(FLT_MAX);
				mesh.meshInfo.AABBMax = glm::vec4(-FLT_MAX);
				for (const MyosotisFW::Meshlet& meshlet : mesh.meshlet)
				{
					mesh.meshInfo.AABBMin = glm::min(mesh.meshInfo.AABBMin, meshlet.meshletInfo.AABBMin);
					mesh.meshInfo.AABBMax = glm::max(mesh.meshInfo.AABBMax, meshlet.meshletInfo.AABBMax);
				}

				// MeshInfo
				mesh.meshInfo.meshletCount = static_cast<uint32_t>(mesh.meshlet.size());
				mesh.meshInfo.vertexFloatCount = static_cast<uint32_t>(mesh.vertex.size()) * (sizeof(MyosotisFW::VertexData) / sizeof(float));
				mesh.meshInfo.indexCount = static_cast<uint32_t>(mesh.vertex.size());

				// [仮] Material
				meshes[chunkIndex].second.basicMaterialInfo.bitFlags = 0;
				meshes[chunkIndex].second.basicMaterialInfo.baseColor = glm::vec4(1.0f);
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

		rawMeshData.vertex.reserve(totalVertices);

		for (uint32_t y = 0; y < textureHeight; y += samplingStep)
		{
			for (uint32_t x = 0; x < textureWidth; x += samplingStep)
			{
				const float heightValue = static_cast<float>(pixels[y * textureWidth + x]) / 255.0f;

				// Position
				glm::vec3 v = glm::vec3(
					static_cast<float>(x) * MyosotisFW::AppInfo::g_terrainScale.x,
					static_cast<float>(y) * MyosotisFW::AppInfo::g_terrainScale.z,
					heightValue * MyosotisFW::AppInfo::g_terrainScale.y
				);

				// Normal (固定: 上向き)
				glm::vec3 n = glm::vec3(0.0f, 1.0f, 0.0f);

				// UV0
				glm::vec2 u0 = glm::vec2(
					static_cast<float>(x) / (textureWidth - 1),
					static_cast<float>(y) / (textureHeight - 1)
				);
				// UV1 (仮)
				glm::vec2 u1 = glm::vec2(0.0f);

				// Color (固定: 白)
				glm::vec4 c = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

				rawMeshData.vertex.insert(rawMeshData.vertex.end(), { v, n, u0, u1, c });
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
		ComputeNormalsFromGrid(rawMeshData.vertex, sampledWidth, sampledHeight);

		return rawMeshData;
	}
}