// Copyright (c) 2025 kong9812
#pragma once
#include <vector>

#include "iglm.h"
#include "Structs.h"

// 順番
// [Vec3]Position
// [Vec4]Position
// [Vec3]Normal
// [Vec2]UV
// [Vec3]Tangent
// [Vec3]Color
// [Vec4]Color

namespace MyosotisFW::System::Render::Shape
{
	enum class PrimitiveGeometryShape
	{
		UNDEFINED,
		Quad,
		Plane,
		Circle,
		Sphere,
	};

	inline Mesh createQuad(const float& size = 1.0f, const glm::vec4& color = { 1.0f, 1.0f, 1.0f, 1.0f }, const glm::vec3& center = { 0.0f, 0.0f, 0.0f })
	{
		float halfSize = size * 0.5f;
		Mesh mesh = {};

		// 頂点データ (x, y, z, w, nx, ny, nz, uvx, uvy, r, g, b, a,)
		mesh.vertex = {
			// 前面 (0, 0, -1)
			-halfSize + center.x, -halfSize + center.y, -halfSize + center.z,  1.0,  0.0,  0.0, -1.0, 0.0, 1.0, color.r, color.g, color.b, color.a,	// 0
			 halfSize + center.x, -halfSize + center.y, -halfSize + center.z,  1.0,  0.0,  0.0, -1.0, 1.0, 1.0, color.r, color.g, color.b, color.a,	// 1
			 halfSize + center.x,  halfSize + center.y, -halfSize + center.z,  1.0,  0.0,  0.0, -1.0, 1.0, 0.0, color.r, color.g, color.b, color.a,	// 2
			-halfSize + center.x,  halfSize + center.y, -halfSize + center.z,  1.0,  0.0,  0.0, -1.0, 0.0, 0.0, color.r, color.g, color.b, color.a,	// 3

			// 背面 (0, 0, 1)
			-halfSize + center.x, -halfSize + center.y,  halfSize + center.z,  1.0,  0.0,  0.0,  1.0, 0.0, 1.0, color.r, color.g, color.b, color.a,	// 4
			 halfSize + center.x, -halfSize + center.y,  halfSize + center.z,  1.0,  0.0,  0.0,  1.0, 1.0, 1.0, color.r, color.g, color.b, color.a,	// 5
			 halfSize + center.x,  halfSize + center.y,  halfSize + center.z,  1.0,  0.0,  0.0,  1.0, 1.0, 0.0, color.r, color.g, color.b, color.a,	// 6
			-halfSize + center.x,  halfSize + center.y,  halfSize + center.z,  1.0,  0.0,  0.0,  1.0, 0.0, 0.0, color.r, color.g, color.b, color.a,	// 7

			// 上面 (0, 1, 0)
			-halfSize + center.x,  halfSize + center.y, -halfSize + center.z,  1.0,  0.0,  1.0,  0.0, 0.0, 1.0, color.r, color.g, color.b, color.a,	// 8
			 halfSize + center.x,  halfSize + center.y, -halfSize + center.z,  1.0,  0.0,  1.0,  0.0, 1.0, 1.0, color.r, color.g, color.b, color.a,	// 9
			 halfSize + center.x,  halfSize + center.y,  halfSize + center.z,  1.0,  0.0,  1.0,  0.0, 1.0, 0.0, color.r, color.g, color.b, color.a,	// 10
			-halfSize + center.x,  halfSize + center.y,  halfSize + center.z,  1.0,  0.0,  1.0,  0.0, 0.0, 0.0, color.r, color.g, color.b, color.a,	// 11

			// 下面 (0, -1, 0)
			-halfSize + center.x, -halfSize + center.y, -halfSize + center.z,  1.0,  0.0, -1.0,  0.0, 0.0, 1.0, color.r, color.g, color.b, color.a,	// 12
			 halfSize + center.x, -halfSize + center.y, -halfSize + center.z,  1.0,  0.0, -1.0,  0.0, 1.0, 1.0, color.r, color.g, color.b, color.a,	// 13
			 halfSize + center.x, -halfSize + center.y,  halfSize + center.z,  1.0,  0.0, -1.0,  0.0, 1.0, 0.0, color.r, color.g, color.b, color.a,	// 14
			-halfSize + center.x, -halfSize + center.y,  halfSize + center.z,  1.0,  0.0, -1.0,  0.0, 0.0, 0.0, color.r, color.g, color.b, color.a,	// 15

			// 右面 (1, 0, 0)
			 halfSize + center.x, -halfSize + center.y, -halfSize + center.z,  1.0,  1.0,  0.0,  0.0, 0.0, 1.0, color.r, color.g, color.b, color.a,	// 16
			 halfSize + center.x,  halfSize + center.y, -halfSize + center.z,  1.0,  1.0,  0.0,  0.0, 1.0, 1.0, color.r, color.g, color.b, color.a,	// 17
			 halfSize + center.x,  halfSize + center.y,  halfSize + center.z,  1.0,  1.0,  0.0,  0.0, 1.0, 0.0, color.r, color.g, color.b, color.a,	// 18
			 halfSize + center.x, -halfSize + center.y,  halfSize + center.z,  1.0,  1.0,  0.0,  0.0, 0.0, 0.0, color.r, color.g, color.b, color.a,	// 19

			 // 左面 (-1, 0, 0)
			 -halfSize + center.x, -halfSize + center.y, -halfSize + center.z, 1.0, -1.0,  0.0,  0.0, 0.0, 1.0, color.r, color.g, color.b, color.a,	// 20
			 -halfSize + center.x,  halfSize + center.y, -halfSize + center.z, 1.0, -1.0,  0.0,  0.0, 1.0, 1.0, color.r, color.g, color.b, color.a,	// 21
			 -halfSize + center.x,  halfSize + center.y,  halfSize + center.z, 1.0, -1.0,  0.0,  0.0, 1.0, 0.0, color.r, color.g, color.b, color.a,	// 22
			 -halfSize + center.x, -halfSize + center.y,  halfSize + center.z, 1.0, -1.0,  0.0,  0.0, 0.0, 0.0, color.r, color.g, color.b, color.a,	// 23
		};
		mesh.index = {
			0,	2,	1,	0,	3,	2,	// 前面
			4,	5,	6,	4,	6,	7,	// 背面
			8,	10,	9,	8,	11,	10,	// 上面
			12, 13,	14,	12,	14,	15,	// 下面
			16, 17,	18,	16,	18,	19,	// 右面
			20, 22,	21,	20,	23,	22	// 左面
		};

		return mesh;
	}

	inline Mesh createPlane(float size = 1.0f, glm::vec4 color = { 1.0f,1.0f,1.0f,1.0f }, glm::vec3 center = { 0.0f,0.0f,0.0f })
	{
		float halfSize = size * 0.5f;
		Mesh mesh = {};
		// 頂点データ (x, y, z, w, r, g, b, a, nx, ny, nz)
		mesh.vertex = {
			// 上面 (0, 1, 0)
			-halfSize + center.x,  halfSize + center.y, -halfSize + center.z,  1.0,  0.0,  1.0,  0.0, 0.0, 1.0, color.r, color.g, color.b, color.a,	// 8
			 halfSize + center.x,  halfSize + center.y, -halfSize + center.z,  1.0,  0.0,  1.0,  0.0, 1.0, 1.0, color.r, color.g, color.b, color.a,	// 9
			 halfSize + center.x,  halfSize + center.y,  halfSize + center.z,  1.0,  0.0,  1.0,  0.0, 1.0, 0.0, color.r, color.g, color.b, color.a,	// 10
			-halfSize + center.x,  halfSize + center.y,  halfSize + center.z,  1.0,  0.0,  1.0,  0.0, 0.0, 0.0, color.r, color.g, color.b, color.a,	// 11
		};
		mesh.index = {
			0,	2,	1,	0,	3,	2,	// 前面
		};
		return mesh;
	}

	inline Mesh createCircle(float size = 1.0f, glm::vec4 color = { 1.0f,1.0f,1.0f,1.0f }, glm::vec3 center = { 0.0f,0.0f,0.0f }, uint32_t side = 12)
	{
		float radius = size * 0.5f;
		Mesh mesh = {};

		ASSERT(side > 2, "circle's side must be more than 2");

		// 頂点データ (x, y, z, w, r, g, b, a, nx, ny, nz)
		float a = glm::two_pi<float>() / static_cast<float>(side);

		mesh.vertex.insert(mesh.vertex.end(), {
			center.x, center.y, center.z, 1.0f,
			0.0f, 0.0f, -1.0f,
			0.0f, 0.0f,
			color.r, color.g, color.b, color.a });
		mesh.vertex.insert(mesh.vertex.end(), {
			center.x + radius * sinf(a), center.y + radius * cosf(a), center.z, 1.0f,
			0.0f, 0.0f, -1.0f,
			0.0f, 0.0f,
			color.r, color.g, color.b, color.a });

		for (uint32_t i = 1; i <= side; i++)
		{
			uint32_t idx = i + 1;
			idx = idx > side ? idx - side : idx;
			mesh.vertex.insert(mesh.vertex.end(), {
				center.x + radius * sinf(a * static_cast<float>(idx)), center.y + radius * cosf(a * static_cast<float>(idx)), center.z, 1.0f,
				0.0f, 0.0f, -1.0f,
				0.0f, 0.0f,
				color.r, color.g, color.b, color.a });
			mesh.index.insert(mesh.index.end(), { 0, i, idx });
		}
		return mesh;
	}

	inline Mesh createSphere(float size = 1.0f, glm::vec4 color = { 1.0f,1.0f,1.0f,1.0f }, glm::vec3 center = { 0.0f,0.0f,0.0f }, uint32_t side = 96)
	{
		float radius = size * 0.5f;
		Mesh mesh = {};
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
					position.x, position.y, position.z, 1.0f,
					normal.x, normal.y, normal.z,
					uv.x, uv.y,
					color.r, color.g, color.b, color.a });
			}
		}

		for (uint32_t i = 0; i < side; i++)
		{
			for (uint32_t j = 0; j < side; j++)
			{
				uint32_t idx = i * (side + 1) + j;
				mesh.index.insert(mesh.index.end(), { idx + 1, idx + side + 1, idx });
				mesh.index.insert(mesh.index.end(), { idx + 1, idx + side + 2, idx + side + 1 });
			}
		}
		return mesh;
	}

	inline Mesh createShape(PrimitiveGeometryShape shape,
		float size = 1.0f,
		glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f },
		glm::vec3 center = { 0.0f, 0.0f, 0.0f })
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
		default:
			ASSERT(false, "Invalid shape");
			return {};
		}
	}
}


