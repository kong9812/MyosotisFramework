// Copyright (c) 2025 kong9812
#pragma once
#include <vector>
#include <unordered_set>

#include "AppInfo.h"
#include "iglm.h"
#include "Logger.h"
#include "Meshlet.h"
#include "Mesh.h"

#include "imeshoptimizer.h"
#include "ixatlas.h"

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
		Cylinder,
		Capsule,
		Max
	};

	inline Mesh createQuad(const float size = 1.0f, const glm::vec4& color = { 1.0f, 1.0f, 1.0f, 1.0f }, const glm::vec3& center = { 0.0f, 0.0f, 0.0f })
	{
		float halfSize = size * 0.5f;
		Mesh mesh = {};

		// vertex
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
		// index
		mesh.index = {
			0,	1,	2,	2,	1,	3,	// 前面
			4,	5,	6,	6,	5,	7,	// 背面
			8,	9,	10,	10,	9,	11,	// 上面
			12, 13,	14,	14,	13,	15,	// 下面
			16, 17,	18,	18,	17,	19,	// 右面
			20, 21,	22,	22,	21,	23	// 左面
		};

		// UV1
		mesh.meshInfo.atlasSize = xatlas::BuildLightmapUV(mesh.vertex, mesh.index);

		// Meshlet
		meshoptimizer::BuildMeshletData(mesh, mesh.index,
			MyosotisFW::AppInfo::g_maxMeshletVertices,
			MyosotisFW::AppInfo::g_maxMeshletPrimitives);

		// AABB
		mesh.meshInfo.AABBMin = glm::vec4(FLT_MAX);
		mesh.meshInfo.AABBMax = glm::vec4(-FLT_MAX);
		for (const Meshlet& meshlet : mesh.meshlet)
		{
			mesh.meshInfo.AABBMin = glm::min(mesh.meshInfo.AABBMin, meshlet.meshletInfo.AABBMin);
			mesh.meshInfo.AABBMax = glm::max(mesh.meshInfo.AABBMax, meshlet.meshletInfo.AABBMax);
		}
		// meshInfo
		mesh.meshInfo.indexCount = static_cast<uint32_t>(mesh.index.size());
		return mesh;
	}

	inline Mesh createPlane(const float size = 1.0f, const glm::vec4& color = { 1.0f,1.0f,1.0f,1.0f }, const glm::vec3& center = { 0.0f,0.0f,0.0f })
	{
		float halfSize = size * 0.5f;
		Mesh mesh = {};

		// vertex
		mesh.vertex = {
			// 前面 (0, 0, -1)
			{ glm::vec3(-halfSize + center.x,  halfSize + center.y, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec4(color) },	// 0
			{ glm::vec3(halfSize + center.x,  halfSize + center.y, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec4(color) },	// 1
			{ glm::vec3(-halfSize + center.x, -halfSize + center.y, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec4(color) },	// 2
			{ glm::vec3(halfSize + center.x, -halfSize + center.y, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec4(color) },	// 3
		};
		// index
		mesh.index = {
			0,	1,	2,	2,	1,	3,	// 前面
		};

		// UV1
		mesh.meshInfo.atlasSize = xatlas::BuildLightmapUV(mesh.vertex, mesh.index);

		// Meshlet
		meshoptimizer::BuildMeshletData(mesh, mesh.index,
			MyosotisFW::AppInfo::g_maxMeshletVertices,
			MyosotisFW::AppInfo::g_maxMeshletPrimitives);

		// AABB
		mesh.meshInfo.AABBMin = glm::vec4(FLT_MAX);
		mesh.meshInfo.AABBMax = glm::vec4(-FLT_MAX);
		for (const Meshlet& meshlet : mesh.meshlet)
		{
			mesh.meshInfo.AABBMin = glm::min(mesh.meshInfo.AABBMin, meshlet.meshletInfo.AABBMin);
			mesh.meshInfo.AABBMax = glm::max(mesh.meshInfo.AABBMax, meshlet.meshletInfo.AABBMax);
		}
		// meshInfo
		mesh.meshInfo.indexCount = static_cast<uint32_t>(mesh.index.size());
		return mesh;
	}

	inline Mesh createCircle(const float size = 1.0f, const glm::vec4& color = { 1.0f,1.0f,1.0f,1.0f }, const glm::vec3& center = { 0.0f,0.0f,0.0f }, const uint32_t side = 24)
	{
		float radius = size * 0.5f;
		Mesh mesh = {};
		ASSERT(side > 2, "circle's side must be more than 2");

		// vertex
		float a = glm::two_pi<float>() / static_cast<float>(side);
		mesh.vertex.insert(mesh.vertex.end(), {
			glm::vec3(center),
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

		// index&vertex
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
			mesh.index.insert(mesh.index.end(), { 0, i, idx });
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
		for (const Meshlet& meshlet : mesh.meshlet)
		{
			mesh.meshInfo.AABBMin = glm::min(mesh.meshInfo.AABBMin, meshlet.meshletInfo.AABBMin);
			mesh.meshInfo.AABBMax = glm::max(mesh.meshInfo.AABBMax, meshlet.meshletInfo.AABBMax);
		}
		// meshInfo
		mesh.meshInfo.indexCount = static_cast<uint32_t>(mesh.index.size());
		return mesh;
	}

	inline Mesh createSphere(const float size = 1.0f, const glm::vec4& color = { 1.0f,1.0f,1.0f,1.0f }, const glm::vec3& center = { 0.0f,0.0f,0.0f }, const uint32_t side = 24)
	{
		float radius = size * 0.5f;
		Mesh mesh = {};
		ASSERT(side > 3, "sphere's side must be more than 3");

		// vertex
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
		// index
		for (uint32_t i = 0; i < side; i++)
		{
			for (uint32_t j = 0; j < side; j++)
			{
				uint32_t idx = i * (side + 1) + j;
				mesh.index.insert(mesh.index.end(), { idx + 1, idx + side + 1, idx });
				mesh.index.insert(mesh.index.end(), { idx + 1, idx + side + 2, idx + side + 1 });
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
		for (const Meshlet& meshlet : mesh.meshlet)
		{
			mesh.meshInfo.AABBMin = glm::min(mesh.meshInfo.AABBMin, meshlet.meshletInfo.AABBMin);
			mesh.meshInfo.AABBMax = glm::max(mesh.meshInfo.AABBMax, meshlet.meshletInfo.AABBMax);
		}
		// meshInfo
		mesh.meshInfo.indexCount = static_cast<uint32_t>(mesh.index.size());
		return mesh;
	}

	inline Mesh createPlaneWithHole(const float size = 1.0f, const glm::vec4& color = { 1.0f,1.0f,1.0f,1.0f }, const glm::vec3& center = { 0.0f,0.0f,0.0f }, const float holeSize = 0.1f)
	{
		ASSERT(size > holeSize, "hole size should be larger than size.");

		float holeHalfSize = holeSize * 0.5f;
		float halfSize = size * 0.5f;
		Mesh mesh = {};
		// vertex
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
		// index
		mesh.index = {
			0,	1,	2,	2,	1,	3,	// 上
			4,	5,	6,	6,	5,	7,	// 左
			8,	9,	10,	10,	9,	11,	// 右
			12,	13,	14,	14,	13,	15,	// 下
		};

		// UV1
		mesh.meshInfo.atlasSize = xatlas::BuildLightmapUV(mesh.vertex, mesh.index);

		// Meshlet
		meshoptimizer::BuildMeshletData(mesh, mesh.index,
			MyosotisFW::AppInfo::g_maxMeshletVertices,
			MyosotisFW::AppInfo::g_maxMeshletPrimitives);

		// AABB
		mesh.meshInfo.AABBMin = glm::vec4(FLT_MAX);
		mesh.meshInfo.AABBMax = glm::vec4(-FLT_MAX);
		for (const Meshlet& meshlet : mesh.meshlet)
		{
			mesh.meshInfo.AABBMin = glm::min(mesh.meshInfo.AABBMin, meshlet.meshletInfo.AABBMin);
			mesh.meshInfo.AABBMax = glm::max(mesh.meshInfo.AABBMax, meshlet.meshletInfo.AABBMax);
		}
		// meshInfo
		mesh.meshInfo.indexCount = static_cast<uint32_t>(mesh.index.size());
		return mesh;
	}

	inline Mesh createCylinder(const float size = 1.0f, const glm::vec4& color = { 1.0f,1.0f,1.0f,1.0f }, const glm::vec3& center = { 0,0,0 }, const uint32_t side = 24)
	{
		Mesh mesh = {};
		ASSERT(side > 2, "cylinder's side must be >= 3");

		float radius = size * 0.5f;
		float height = size;

		mesh.index.reserve(static_cast<size_t>(side * 12));

		// side vertex
		for (uint32_t i = 0; i <= side; i++)
		{
			float t = glm::two_pi<float>() * (float)i / (float)side;
			float x = cosf(t);
			float z = sinf(t);

			glm::vec3 normal = { x, 0.0f, z };

			// up
			mesh.vertex.push_back({
				glm::vec3(center.x + x * radius, center.y + height * 0.5f, center.z + z * radius),
				normal,
				glm::vec2((float)i / side, 0.0f),
				glm::vec2(0.0f),
				color
				});

			// down
			mesh.vertex.push_back({
				glm::vec3(center.x + x * radius, center.y - height * 0.5f, center.z + z * radius),
				normal,
				glm::vec2((float)i / side, 1.0f),
				glm::vec2(0.0f),
				color
				});
		}
		// side index
		for (uint32_t i = 0; i < side; i++)
		{
			uint32_t top0 = i * 2;
			uint32_t bot0 = top0 + 1;
			uint32_t top1 = top0 + 2;
			uint32_t bot1 = top0 + 3;

			mesh.index.insert(mesh.index.end(), {
				top0, top1, bot0,
				top1, bot1, bot0
				});
		}

		// up vertex
		uint32_t topCenterIdx = (uint32_t)mesh.vertex.size();
		mesh.vertex.push_back({
			glm::vec3(center.x, center.y + height * 0.5f, center.z),
			glm::vec3(0.0f, 1.0f, 0.0f),
			glm::vec2(0.5f,0.5f),
			glm::vec2(0.0f),
			color
			});
		uint32_t topStart = topCenterIdx + 1;
		for (uint32_t i = 0; i <= side; i++)
		{
			float t = glm::two_pi<float>() * (float)i / (float)side;
			float x = cosf(t);
			float z = sinf(t);

			mesh.vertex.push_back({
				glm::vec3(center.x + x * radius, center.y + height * 0.5f, center.z + z * radius),
				glm::vec3(0.0f, 1.0f, 0.0f),
				glm::vec2(x * 0.5f + 0.5f, z * 0.5f + 0.5f),
				glm::vec2(0.0f),
				color
				});
		}
		// up index
		for (uint32_t i = 0; i < side; i++)
		{
			mesh.index.insert(mesh.index.end(), { topCenterIdx, topStart + i + 1, topStart + i });
		}

		// down vertex
		uint32_t bottomCenterIdx = (uint32_t)mesh.vertex.size();
		mesh.vertex.push_back({
			glm::vec3(center.x, center.y - height * 0.5f, center.z),
			glm::vec3(0.0f, -1.0f, 0.0f),
			glm::vec2(0.5f,0.5f),
			glm::vec2(0.0f),
			color
			});
		uint32_t bottomStart = bottomCenterIdx + 1;
		for (uint32_t i = 0; i <= side; i++)
		{
			float t = glm::two_pi<float>() * (float)i / (float)side;
			float x = cosf(t);
			float z = sinf(t);

			mesh.vertex.push_back({
				glm::vec3(center.x + x * radius, center.y - height * 0.5f, center.z + z * radius),
				glm::vec3(0.0f, -1.0f, 0.0f),
				glm::vec2(x * 0.5f + 0.5f, z * 0.5f + 0.5f),
				glm::vec2(0.0f),
				color
				});
		}
		// down index
		for (uint32_t i = 0; i < side; i++)
		{
			mesh.index.insert(mesh.index.end(), { bottomCenterIdx, bottomStart + i, bottomStart + i + 1 });
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
		for (const Meshlet& meshlet : mesh.meshlet)
		{
			mesh.meshInfo.AABBMin = glm::min(mesh.meshInfo.AABBMin, meshlet.meshletInfo.AABBMin);
			mesh.meshInfo.AABBMax = glm::max(mesh.meshInfo.AABBMax, meshlet.meshletInfo.AABBMax);
		}
		// meshInfo
		mesh.meshInfo.indexCount = static_cast<uint32_t>(mesh.index.size());
		return mesh;
	}

	inline Mesh createCapsule(const float size = 1.0f, const glm::vec4& color = { 1,1,1,1 }, const glm::vec3& center = { 0,0,0 }, const uint32_t side = 24, const uint32_t ring = 12)
	{
		Mesh mesh = {};
		ASSERT(side > 2, "cylinder's side must be >= 3");

		float radius = size * 0.5f;
		float height = size;

		mesh.index.reserve(static_cast<size_t>(side * 12));

		// side vertex
		for (uint32_t i = 0; i <= side; i++)
		{
			float t = glm::two_pi<float>() * (float)i / (float)side;
			float x = cosf(t);
			float z = sinf(t);

			glm::vec3 normal = { x, 0.0f, z };

			// up
			mesh.vertex.push_back({
				glm::vec3(center.x + x * radius, center.y + height * 0.5f, center.z + z * radius),
				normal,
				glm::vec2((float)i / side, 0.0f),
				glm::vec2(0.0f),
				color
				});

			// down
			mesh.vertex.push_back({
				glm::vec3(center.x + x * radius, center.y - height * 0.5f, center.z + z * radius),
				normal,
				glm::vec2((float)i / side, 1.0f),
				glm::vec2(0.0f),
				color
				});
		}
		// side index
		for (uint32_t i = 0; i < side; i++)
		{
			uint32_t top0 = i * 2;
			uint32_t bot0 = top0 + 1;
			uint32_t top1 = top0 + 2;
			uint32_t bot1 = top0 + 3;

			mesh.index.insert(mesh.index.end(), {
				top0, top1, bot0,
				top1, bot1, bot0
				});
		}

		// up vertex (hemisphere)
		uint32_t topStart = (uint32_t)mesh.vertex.size();
		for (uint32_t iz = 0; iz <= side / 2; iz++)
		{
			float vz = (float)iz / (side / 2.0f);
			float phi = glm::half_pi<float>() * vz;
			float y = cosf(phi) * radius + height * 0.5f;
			float r = sinf(phi) * radius;

			for (uint32_t ix = 0; ix <= side; ix++)
			{
				float t = glm::two_pi<float>() * (float)ix / (float)side;
				float x = cosf(t) * r;
				float z = sinf(t) * r;

				glm::vec3 pos = glm::vec3(center.x + x, center.y + y, center.z + z);
				glm::vec3 normal = glm::normalize(glm::vec3(x, y - height * 0.5f, z));

				mesh.vertex.push_back({
					pos,
					normal,
					glm::vec2((float)ix / side, vz),
					glm::vec2(0.0f),
					color
					});
			}
		}
		// up index
		for (uint32_t iz = 0; iz < side / 2; iz++)
		{
			for (uint32_t ix = 0; ix < side; ix++)
			{
				uint32_t i0 = topStart + iz * (side + 1) + ix;
				uint32_t i1 = i0 + 1;
				uint32_t i2 = i0 + (side + 1);
				uint32_t i3 = i2 + 1;

				mesh.index.insert(mesh.index.end(), { i0, i1, i2 });
				mesh.index.insert(mesh.index.end(), { i1, i3, i2 });
			}
		}

		// down vertex (hemisphere)
		uint32_t bottomStart = (uint32_t)mesh.vertex.size();
		for (uint32_t iz = 0; iz <= side / 2; iz++)
		{
			float vz = (float)iz / (side / 2.0f);
			float phi = glm::half_pi<float>() * vz;
			float y = -cosf(phi) * radius - height * 0.5f;
			float r = sinf(phi) * radius;

			for (uint32_t ix = 0; ix <= side; ix++)
			{
				float t = glm::two_pi<float>() * (float)ix / (float)side;
				float x = cosf(t) * r;
				float z = sinf(t) * r;

				glm::vec3 pos = glm::vec3(center.x + x, center.y + y, center.z + z);
				glm::vec3 normal = glm::normalize(glm::vec3(x, y + height * 0.5f, z));

				mesh.vertex.push_back({
					pos,
					normal,
					glm::vec2((float)ix / side, 1.0f - vz),
					glm::vec2(0.0f),
					color
					});
			}
		}
		// down index
		for (uint32_t iz = 0; iz < side / 2; iz++)
		{
			for (uint32_t ix = 0; ix < side; ix++)
			{
				uint32_t i0 = bottomStart + iz * (side + 1) + ix;
				uint32_t i1 = i0 + 1;
				uint32_t i2 = i0 + (side + 1);
				uint32_t i3 = i2 + 1;

				mesh.index.insert(mesh.index.end(), { i1, i0, i2 });
				mesh.index.insert(mesh.index.end(), { i1, i2, i3 });
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
		for (const Meshlet& meshlet : mesh.meshlet)
		{
			mesh.meshInfo.AABBMin = glm::min(mesh.meshInfo.AABBMin, meshlet.meshletInfo.AABBMin);
			mesh.meshInfo.AABBMax = glm::max(mesh.meshInfo.AABBMax, meshlet.meshletInfo.AABBMax);
		}
		// meshInfo
		mesh.meshInfo.indexCount = static_cast<uint32_t>(mesh.index.size());
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
		case PrimitiveGeometryShape::Cylinder:
			return createCylinder(size, color, center);
		case PrimitiveGeometryShape::Capsule:
			return createCapsule(size, color, center);
		default:
			ASSERT(false, "Invalid shape");
			return {};
		}
	}
}


