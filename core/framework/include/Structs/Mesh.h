// Copyright (c) 2025 kong9812
#pragma once
#include <vector>
#include "Meshlet.h"
#include "MeshInfo.h"
#include "VertexData.h"
#include "Buffer.h"
#include "ClassPointer.h"

namespace MyosotisFW
{
	struct Mesh
	{
		// Info
		MeshInfo meshInfo;

		std::vector<uint32_t> index;
		std::vector<VertexData> vertex;
		std::vector<Meshlet> meshlet;

		// Buffer
		Buffer vertexBuffer;
		Buffer indexBuffer;

		// BLAS
		mutable uint32_t blasID;
	};
	TYPEDEF_SHARED_PTR(Mesh);						// RenderResoureces保持用
	using MeshHandle = std::weak_ptr<const Mesh>;	// 外部参照用

	// 複数メッシュ管理
	using Meshes = std::vector<Mesh_ptr>;			// RenderResoureces保持用
	using MeshesHandle = std::vector<MeshHandle>;	// 外部参照用
}
