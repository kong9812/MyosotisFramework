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
	inline Mesh createQuad(float size = 1.0f, glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f }, glm::vec3 center = { 0.0f, 0.0f, 0.0f })
	{
		float halfSize = size * 0.5f;
		Mesh mesh = {};

		// 頂点データ (x, y, z, w, r, g, b, a, nx, ny, nz)
		mesh.vertex = {
			// 前面 (0, 0, -1)
			-halfSize + center.x, -halfSize + center.y, -halfSize + center.z,  1.0,  0.0,  0.0, -1.0, color.r, color.g, color.b, color.a,	// 0
			 halfSize + center.x, -halfSize + center.y, -halfSize + center.z,  1.0,  0.0,  0.0, -1.0, color.r, color.g, color.b, color.a,	// 1
			 halfSize + center.x,  halfSize + center.y, -halfSize + center.z,  1.0,  0.0,  0.0, -1.0, color.r, color.g, color.b, color.a,	// 2
			-halfSize + center.x,  halfSize + center.y, -halfSize + center.z,  1.0,  0.0,  0.0, -1.0, color.r, color.g, color.b, color.a,	// 3

			// 背面 (0, 0, 1)
			-halfSize + center.x, -halfSize + center.y,  halfSize + center.z,  1.0,  0.0,  0.0,  1.0, color.r, color.g, color.b, color.a,	// 4
			 halfSize + center.x, -halfSize + center.y,  halfSize + center.z,  1.0,  0.0,  0.0,  1.0, color.r, color.g, color.b, color.a,	// 5
			 halfSize + center.x,  halfSize + center.y,  halfSize + center.z,  1.0,  0.0,  0.0,  1.0, color.r, color.g, color.b, color.a,	// 6
			-halfSize + center.x,  halfSize + center.y,  halfSize + center.z,  1.0,  0.0,  0.0,  1.0, color.r, color.g, color.b, color.a,	// 7

			// 上面 (0, 1, 0)
			-halfSize + center.x,  halfSize + center.y, -halfSize + center.z,  1.0,  0.0,  1.0,  0.0, color.r, color.g, color.b, color.a,	// 8
			 halfSize + center.x,  halfSize + center.y, -halfSize + center.z,  1.0,  0.0,  1.0,  0.0, color.r, color.g, color.b, color.a,	// 9
			 halfSize + center.x,  halfSize + center.y,  halfSize + center.z,  1.0,  0.0,  1.0,  0.0, color.r, color.g, color.b, color.a,	// 10
			-halfSize + center.x,  halfSize + center.y,  halfSize + center.z,  1.0,  0.0,  1.0,  0.0, color.r, color.g, color.b, color.a,	// 11

			// 下面 (0, -1, 0)
			-halfSize + center.x, -halfSize + center.y, -halfSize + center.z,  1.0,  0.0, -1.0,  0.0, color.r, color.g, color.b, color.a,	// 12
			 halfSize + center.x, -halfSize + center.y, -halfSize + center.z,  1.0,  0.0, -1.0,  0.0, color.r, color.g, color.b, color.a,	// 13
			 halfSize + center.x, -halfSize + center.y,  halfSize + center.z,  1.0,  0.0, -1.0,  0.0, color.r, color.g, color.b, color.a,	// 14
			-halfSize + center.x, -halfSize + center.y,  halfSize + center.z,  1.0,  0.0, -1.0,  0.0, color.r, color.g, color.b, color.a,	// 15

			// 右面 (1, 0, 0)
			 halfSize + center.x, -halfSize + center.y, -halfSize + center.z,  1.0,  1.0,  0.0,  0.0, color.r, color.g, color.b, color.a,	// 16
			 halfSize + center.x,  halfSize + center.y, -halfSize + center.z,  1.0,  1.0,  0.0,  0.0, color.r, color.g, color.b, color.a,	// 17
			 halfSize + center.x,  halfSize + center.y,  halfSize + center.z,  1.0,  1.0,  0.0,  0.0, color.r, color.g, color.b, color.a,	// 18
			 halfSize + center.x, -halfSize + center.y,  halfSize + center.z,  1.0,  1.0,  0.0,  0.0, color.r, color.g, color.b, color.a,	// 19

			 // 左面 (-1, 0, 0)
			 -halfSize + center.x, -halfSize + center.y, -halfSize + center.z, 1.0, -1.0,  0.0,  0.0, color.r, color.g, color.b, color.a,	// 20
			 -halfSize + center.x,  halfSize + center.y, -halfSize + center.z, 1.0, -1.0,  0.0,  0.0, color.r, color.g, color.b, color.a,	// 21
			 -halfSize + center.x,  halfSize + center.y,  halfSize + center.z, 1.0, -1.0,  0.0,  0.0, color.r, color.g, color.b, color.a,	// 22
			 -halfSize + center.x, -halfSize + center.y,  halfSize + center.z, 1.0, -1.0,  0.0,  0.0, color.r, color.g, color.b, color.a,	// 23
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
}


