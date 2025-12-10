// Copyright (c) 2025 kong9812
#pragma once
#include <vector>
#include <unordered_set>

#include "AppInfo.h"
#include "iglm.h"
#include "Logger.h"
#include "Meshlet.h"
#include "Mesh.h"

// 順番
// [Vec3]Position
// [Vec4]Position
// [Vec3]Normal
// [Vec2]UV
// [Vec3]Tangent
// [Vec3]Color
// [Vec4]Color

// 座標系
// Y ↑+
// Y ↓-
// X ←-
// X →+
// Z ↑+
// Z ↓-

namespace MyosotisFW::System::Render::Shape
{
	enum class PrimitiveGeometryShape
	{
		UNDEFINED = -1,
		Quad,
		Plane,
		Circle,
		Sphere,
		PlaneWithHole,
		Max
	};

	inline Mesh createQuad(const float size = 1.0f, const glm::vec4& color = { 1.0f, 1.0f, 1.0f, 1.0f }, const glm::vec3& center = { 0.0f, 0.0f, 0.0f })
	{
		float halfSize = size * 0.5f;
		Mesh mesh = {};
		Meshlet meshlet = {};
		std::unordered_set<uint32_t> uniqueVertexIndices{};

		// 頂点データ (x, y, z, w, nx, ny, nz, uvx, uvy, r, g, b, a,)
		mesh.vertex = {
			// 前面 (0, 0, -1)
			{ glm::vec3(-halfSize + center.x,  halfSize + center.y, -halfSize + center.z),  glm::vec3(0.0f,  0.0f, -1.0f), glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec4(color) },	// 3
			{ glm::vec3(halfSize + center.x,  halfSize + center.y, -halfSize + center.z),  glm::vec3(0.0f,  0.0f, -1.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec4(color) },	// 2
			{ glm::vec3(-halfSize + center.x, -halfSize + center.y, -halfSize + center.z),  glm::vec3(0.0f,  0.0f, -1.0f), glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec4(color) },	// 0
			{ glm::vec3(halfSize + center.x, -halfSize + center.y, -halfSize + center.z),  glm::vec3(0.0f,  0.0f, -1.0f), glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec4(color) },	// 1

			// 背面 (0, 0, 1)
			{ glm::vec3(halfSize + center.x,  halfSize + center.y,  halfSize + center.z),  glm::vec3(0.0f,  0.0f,  1.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec4(color) },	// 6
			{ glm::vec3(-halfSize + center.x,  halfSize + center.y,  halfSize + center.z),  glm::vec3(0.0f,  0.0f,  1.0f), glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec4(color) },	// 7
			{ glm::vec3(halfSize + center.x, -halfSize + center.y,  halfSize + center.z),  glm::vec3(0.0f,  0.0f,  1.0f), glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec4(color) },	// 5
			{ glm::vec3(-halfSize + center.x, -halfSize + center.y,  halfSize + center.z),  glm::vec3(0.0f,  0.0f,  1.0f), glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec4(color) },	// 4

			// 上面 (0, 1, 0)
			{ glm::vec3(-halfSize + center.x,  halfSize + center.y,  halfSize + center.z),  glm::vec3(0.0f,  1.0f,  0.0f), glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec4(color) }, // 11
			{ glm::vec3(halfSize + center.x,  halfSize + center.y,  halfSize + center.z),  glm::vec3(0.0f,  1.0f,  0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec4(color) }, // 10
			{ glm::vec3(-halfSize + center.x,  halfSize + center.y, -halfSize + center.z),  glm::vec3(0.0f,  1.0f,  0.0f), glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec4(color) }, // 8
			{ glm::vec3(halfSize + center.x,  halfSize + center.y, -halfSize + center.z),  glm::vec3(0.0f,  1.0f,  0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec4(color) }, // 9

			// 下面 (0, -1, 0)
			{ glm::vec3(halfSize + center.x, -halfSize + center.y,  halfSize + center.z),  glm::vec3(0.0f, -1.0f,  0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec4(color) }, // 14
			{ glm::vec3(-halfSize + center.x, -halfSize + center.y,  halfSize + center.z),  glm::vec3(0.0f, -1.0f,  0.0f), glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec4(color) }, // 15
			{ glm::vec3(halfSize + center.x, -halfSize + center.y, -halfSize + center.z),  glm::vec3(0.0f, -1.0f,  0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec4(color) }, // 13
			{ glm::vec3(-halfSize + center.x, -halfSize + center.y, -halfSize + center.z),  glm::vec3(0.0f, -1.0f,  0.0f), glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec4(color) }, // 12

			// 右面 (1, 0, 0)
			{ glm::vec3(halfSize + center.x,  halfSize + center.y, -halfSize + center.z), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec4(color) }, // 17
			{ glm::vec3(halfSize + center.x,  halfSize + center.y,  halfSize + center.z), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec4(color) }, // 18
			{ glm::vec3(halfSize + center.x, -halfSize + center.y, -halfSize + center.z), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec4(color) }, // 16
			{ glm::vec3(halfSize + center.x, -halfSize + center.y,  halfSize + center.z), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec4(color) }, // 19

			// 左面 (-1, 0, 0)
			{ glm::vec3(-halfSize + center.x, -halfSize + center.y, -halfSize + center.z), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec4(color) }, // 20
			{ glm::vec3(-halfSize + center.x, -halfSize + center.y,  halfSize + center.z), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec4(color) }, // 23
			{ glm::vec3(-halfSize + center.x,  halfSize + center.y, -halfSize + center.z), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec4(color) }, // 21
			{ glm::vec3(-halfSize + center.x,  halfSize + center.y,  halfSize + center.z), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec4(color) }, // 22
		};
		meshlet.primitives = {
			0,	1,	2,	2,	1,	3,	// 前面
			4,	5,	6,	6,	5,	7,	// 背面
			8,	9,	10,	10,	9,	11,	// 上面
			12, 13,	14,	14,	13,	15,	// 下面
			16, 17,	18,	18,	17,	19,	// 右面
			20, 21,	22,	22,	21,	23	// 左面
		};
		for (uint32_t index : meshlet.primitives)
		{
			if (uniqueVertexIndices.insert(index).second)
			{
				meshlet.uniqueIndex.push_back(index);
			}
			else
			{
				auto it = std::find(meshlet.uniqueIndex.begin(), meshlet.uniqueIndex.end(), index);
				uint32_t indexInUnique = std::distance(meshlet.uniqueIndex.begin(), it);
				index = indexInUnique;
			}
		}
		meshlet.meshletInfo.AABBMin = glm::vec4(center - glm::vec3(halfSize), 0.0f);
		meshlet.meshletInfo.AABBMax = glm::vec4(center + glm::vec3(halfSize), 0.0f);
		mesh.meshlet.push_back(meshlet);
		mesh.meshInfo.AABBMin = meshlet.meshletInfo.AABBMin;
		mesh.meshInfo.AABBMax = meshlet.meshletInfo.AABBMax;
		return mesh;
	}

	inline Mesh createPlane(const float size = 1.0f, const glm::vec4& color = { 1.0f,1.0f,1.0f,1.0f }, const glm::vec3& center = { 0.0f,0.0f,0.0f })
	{
		float halfSize = size * 0.5f;
		Mesh mesh = {};
		Meshlet meshlet = {};
		std::unordered_set<uint32_t> uniqueVertexIndices{};
		// 頂点データ (x, y, z, w, r, g, b, a, nx, ny, nz)
		mesh.vertex = {
			// 前面 (0, 0, -1)
			{ glm::vec3(-halfSize + center.x,  halfSize + center.y, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec4(color) },	// 0
			{ glm::vec3(halfSize + center.x,  halfSize + center.y, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec4(color) },	// 1
			{ glm::vec3(-halfSize + center.x, -halfSize + center.y, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec4(color) },	// 2
			{ glm::vec3(halfSize + center.x, -halfSize + center.y, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec4(color) },	// 3
		};
		meshlet.primitives = {
			0,	1,	2,	2,	1,	3,	// 前面
		};
		for (uint32_t index : meshlet.primitives)
		{
			if (uniqueVertexIndices.insert(index).second)
			{
				meshlet.uniqueIndex.push_back(index);
			}
			else
			{
				auto it = std::find(meshlet.uniqueIndex.begin(), meshlet.uniqueIndex.end(), index);
				uint32_t indexInUnique = std::distance(meshlet.uniqueIndex.begin(), it);
				index = indexInUnique;
			}
		}
		meshlet.meshletInfo.AABBMin = glm::vec4(center - glm::vec3(halfSize, halfSize, 0.0f), 0.0f);
		meshlet.meshletInfo.AABBMax = glm::vec4(center + glm::vec3(halfSize, halfSize, 0.0f), 0.0f);
		mesh.meshlet.push_back(meshlet);
		mesh.meshInfo.AABBMin = meshlet.meshletInfo.AABBMin;
		mesh.meshInfo.AABBMax = meshlet.meshletInfo.AABBMax;
		return mesh;
	}

	inline Mesh createCircle(const float size = 1.0f, const glm::vec4& color = { 1.0f,1.0f,1.0f,1.0f }, const glm::vec3& center = { 0.0f,0.0f,0.0f }, const uint32_t side = 12)
	{
		float radius = size * 0.5f;
		Mesh mesh = {};
		Meshlet meshlet = {};
		std::unordered_set<uint32_t> uniqueVertexIndices{};
		ASSERT(side > 2, "circle's side must be more than 2");

		// 頂点データ (x, y, z, w, r, g, b, a, nx, ny, nz)
		float a = glm::two_pi<float>() / static_cast<float>(side);

		mesh.vertex.insert(mesh.vertex.end(), {
			glm::vec3(center.x, center.y, center.z),
			glm::vec3(0.0f, 0.0f, -1.0f),
			glm::vec2(0.0f, 0.0f),
			glm::vec2(0.0f, 0.0f),
			glm::vec4(color) });
		mesh.vertex.insert(mesh.vertex.end(), {
			glm::vec3(center.x + radius * sinf(a), center.y + radius * cosf(a), center.z),
			glm::vec3(0.0f, 0.0f, -1.0f),
			glm::vec2(0.0f, 0.0f),
			glm::vec2(0.0f, 0.0f),
			glm::vec4(color) });

		for (uint32_t i = 1; i <= side; i++)
		{
			uint32_t idx = i + 1;
			idx = idx > side ? idx - side : idx;
			mesh.vertex.insert(mesh.vertex.end(), {
				glm::vec3(center.x + radius * sinf(a * static_cast<float>(idx)), center.y + radius * cosf(a * static_cast<float>(idx)), center.z),
				glm::vec3(0.0f, 0.0f, -1.0f),
				glm::vec2(0.0f, 0.0f),
				glm::vec2(0.0f, 0.0f),
				glm::vec4(color) });
			meshlet.primitives.insert(meshlet.primitives.end(), { 0, i, idx });
		}
		for (uint32_t index : meshlet.primitives)
		{
			if (uniqueVertexIndices.insert(index).second)
			{
				meshlet.uniqueIndex.push_back(index);
			}
			else
			{
				auto it = std::find(meshlet.uniqueIndex.begin(), meshlet.uniqueIndex.end(), index);
				uint32_t indexInUnique = std::distance(meshlet.uniqueIndex.begin(), it);
				index = indexInUnique;
			}
		}
		meshlet.meshletInfo.AABBMin = glm::vec4(center - glm::vec3(radius, radius, 0.0f), 0.0f);
		meshlet.meshletInfo.AABBMax = glm::vec4(center + glm::vec3(radius, radius, 0.0f), 0.0f);
		mesh.meshlet.push_back(meshlet);
		mesh.meshInfo.AABBMin = meshlet.meshletInfo.AABBMin;
		mesh.meshInfo.AABBMax = meshlet.meshletInfo.AABBMax;
		ASSERT(meshlet.primitives.size() < AppInfo::g_maxMeshletPrimitives, "Over MeshletPrimitives count!");
		ASSERT(meshlet.uniqueIndex.size() < AppInfo::g_maxMeshletVertices, "Over MeshletVertices count!");
		return mesh;
	}

	// todo.今は一つのMeshletにまとめているので、壊れてる…
	inline Mesh createSphere(const float size = 1.0f, const glm::vec4& color = { 1.0f,1.0f,1.0f,1.0f }, const glm::vec3& center = { 0.0f,0.0f,0.0f }, const uint32_t side = 4)
	{
		float radius = size * 0.5f;
		Mesh mesh = {};
		Meshlet meshlet = {};
		std::unordered_set<uint32_t> uniqueVertexIndices{};
		ASSERT(side > 3, "sphere's side must be more than 3");

		// 頂点データ (x, y, z, w, r, g, b, a, uvx, uvy, nx, ny, nz)
		for (uint32_t i = 0; i <= side; i++)
		{
			float phi = glm::pi<float>() * static_cast<float>(i) / static_cast<float>(side);

			for (uint32_t j = 0; j <= side; j++)
			{
				float therte = glm::two_pi<float>() * static_cast<float>(j) / static_cast<float>(side);
				glm::vec3 position = {
					center.x + radius * sinf(phi) * cosf(therte),
					center.y + radius * cosf(phi),
					center.z + radius * sinf(phi) * sinf(therte)
				};
				glm::vec3 normal = glm::normalize(position - center);
				glm::vec2 uv = { static_cast<float>(j) / static_cast<float>(side), static_cast<float>(i) / static_cast<float>(side) };

				mesh.vertex.insert(mesh.vertex.end(), {
					glm::vec3(position.x, position.y, position.z),
					glm::vec3(normal.x, normal.y, normal.z),
					glm::vec2(uv.x, uv.y),
					glm::vec2(uv.x, uv.y),
					glm::vec4(color) });
			}
		}

		for (uint32_t i = 0; i < side; i++)
		{
			for (uint32_t j = 0; j < side; j++)
			{
				uint32_t idx = i * (side + 1) + j;
				meshlet.primitives.insert(meshlet.primitives.end(), { idx + 1, idx + side + 1, idx });
				meshlet.primitives.insert(meshlet.primitives.end(), { idx + 1, idx + side + 2, idx + side + 1 });
			}
		}
		for (uint32_t index : meshlet.primitives)
		{
			if (uniqueVertexIndices.insert(index).second)
			{
				meshlet.uniqueIndex.push_back(index);
			}
			else
			{
				auto it = std::find(meshlet.uniqueIndex.begin(), meshlet.uniqueIndex.end(), index);
				uint32_t indexInUnique = std::distance(meshlet.uniqueIndex.begin(), it);
				index = indexInUnique;
			}
		}
		meshlet.meshletInfo.AABBMin = glm::vec4(center - glm::vec3(radius, radius, 0.0f), 0.0f);
		meshlet.meshletInfo.AABBMax = glm::vec4(center + glm::vec3(radius, radius, 0.0f), 0.0f);
		mesh.meshlet.push_back(meshlet);
		mesh.meshInfo.AABBMin = meshlet.meshletInfo.AABBMin;
		mesh.meshInfo.AABBMax = meshlet.meshletInfo.AABBMax;
		ASSERT(meshlet.primitives.size() < AppInfo::g_maxMeshletPrimitives, "Over MeshletPrimitives count!");
		ASSERT(meshlet.uniqueIndex.size() < AppInfo::g_maxMeshletVertices, "Over MeshletVertices count!");
		return mesh;
	}

	inline Mesh createPlaneWithHole(const float size = 1.0f, const glm::vec4& color = { 1.0f,1.0f,1.0f,1.0f }, const glm::vec3& center = { 0.0f,0.0f,0.0f }, const float holeSize = 0.1f)
	{
		ASSERT(size > holeSize, "hole size should be larger than size.");

		float holeHalfSize = holeSize * 0.5f;
		float halfSize = size * 0.5f;
		Mesh mesh = {};
		Meshlet meshlet = {};
		std::unordered_set<uint32_t> uniqueVertexIndices{};
		// 頂点データ (x, y, z, w, r, g, b, a, nx, ny, nz)
		mesh.vertex = {
			// 上 (0, 0, -1)
			{ glm::vec3(-halfSize + center.x, halfSize + center.y, 0.0f), glm::vec3(0.0f, 0.0f, -1.0), glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec4(color) },			// 0
			{ glm::vec3(halfSize + center.x, halfSize + center.y, 0.0f), glm::vec3(0.0f, 0.0f, -1.0), glm::vec2(1.0f, 0.0f),glm::vec2(1.0f, 0.0f), glm::vec4(color) },				// 1
			{ glm::vec3(-halfSize + center.x, holeHalfSize + center.y, 0.0f), glm::vec3(0.0f, 0.0f, -1.0), glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec4(color) },		// 2
			{ glm::vec3(halfSize + center.x, holeHalfSize + center.y, 0.0f), glm::vec3(0.0f, 0.0f, -1.0), glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec4(color) },			// 3

			// 左(0, 0, -1)
			{ glm::vec3(-halfSize + center.x, holeHalfSize + center.y, 0.0f), glm::vec3(0.0f, 0.0f, -1.0), glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec4(color) },		// 4
			{ glm::vec3(-holeHalfSize + center.x, holeHalfSize + center.y, 0.0f), glm::vec3(0.0f, 0.0f, -1.0), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec4(color) },	// 5
			{ glm::vec3(-halfSize + center.x, -holeHalfSize + center.y, 0.0f), glm::vec3(0.0f, 0.0f, -1.0), glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec4(color) },		// 6
			{ glm::vec3(-holeHalfSize + center.x, -holeHalfSize + center.y, 0.0f), glm::vec3(0.0f, 0.0f, -1.0), glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec4(color) },	// 7

			// 右(0, 0, -1)
			{ glm::vec3(holeHalfSize + center.x, holeHalfSize + center.y, 0.0f), glm::vec3(0.0f, 0.0f, -1.0), glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec4(color) },		// 8
			{ glm::vec3(halfSize + center.x, holeHalfSize + center.y, 0.0f), glm::vec3(0.0f, 0.0f, -1.0), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec4(color) },			// 9
			{ glm::vec3(holeHalfSize + center.x, -holeHalfSize + center.y, 0.0f), glm::vec3(0.0f, 0.0f, -1.0), glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec4(color) },	// 10
			{ glm::vec3(halfSize + center.x, -holeHalfSize + center.y, 0.0f), glm::vec3(0.0f, 0.0f, -1.0), glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec4(color) },		// 11

			// 下(0, 0, -1)
			{ glm::vec3(-halfSize + center.x, -holeHalfSize + center.y, 0.0f), glm::vec3(0.0f, 0.0f, -1.0), glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec4(color) },		// 12
			{ glm::vec3(halfSize + center.x, -holeHalfSize + center.y, 0.0f), glm::vec3(0.0f, 0.0f, -1.0), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec4(color) },		// 13
			{ glm::vec3(-halfSize + center.x, -halfSize + center.y, 0.0f), glm::vec3(0.0f, 0.0f, -1.0),glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec4(color) },			// 14
			{ glm::vec3(halfSize + center.x, -halfSize + center.y, 0.0f), glm::vec3(0.0f, 0.0f, -1.0), glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec4(color) },			// 15

		};
		meshlet.primitives = {
			0,	1,	2,	2,	1,	3,	// 上
			4,	5,	6,	6,	5,	7,	// 左
			8,	9,	10,	10,	9,	11,	// 右
			12,	13,	14,	14,	13,	15,	// 下
		};
		for (uint32_t index : meshlet.primitives)
		{
			if (uniqueVertexIndices.insert(index).second)
			{
				meshlet.uniqueIndex.push_back(index);
			}
			else
			{
				auto it = std::find(meshlet.uniqueIndex.begin(), meshlet.uniqueIndex.end(), index);
				uint32_t indexInUnique = std::distance(meshlet.uniqueIndex.begin(), it);
				index = indexInUnique;
			}
		}
		meshlet.meshletInfo.AABBMin = glm::vec4(center - glm::vec3(halfSize, halfSize, 0.0f), 0.0f);
		meshlet.meshletInfo.AABBMax = glm::vec4(center + glm::vec3(halfSize, halfSize, 0.0f), 0.0f);
		mesh.meshlet.push_back(meshlet);
		mesh.meshInfo.AABBMin = meshlet.meshletInfo.AABBMin;
		mesh.meshInfo.AABBMax = meshlet.meshletInfo.AABBMax;
		return mesh;
	}

	inline Mesh createShape(const PrimitiveGeometryShape& shape,
		const float size = 1.0f,
		const glm::vec4& color = { 1.0f, 1.0f, 1.0f, 1.0f },
		const glm::vec3& center = { 0.0f, 0.0f, 0.0f })
	{
		switch (shape)
		{
		case PrimitiveGeometryShape::Quad:
			return createQuad(size, color, center);
		case PrimitiveGeometryShape::Plane:
			return createPlane(size, color, center);
		case PrimitiveGeometryShape::Circle:
			return createCircle(size, color, center);
		case PrimitiveGeometryShape::Sphere:
			return createSphere(size, color, center);
		case PrimitiveGeometryShape::PlaneWithHole:
			return createPlaneWithHole(size, color, center);
		default:
			ASSERT(false, "Invalid shape");
			return {};
		}
	}
}


