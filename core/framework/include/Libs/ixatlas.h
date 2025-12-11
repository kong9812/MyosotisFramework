// Copyright (c) 2025 kong9812
// For include xatlas
#pragma once
#include <xatlas.h>
#include <vector>
#include "Logger.h"
#include "Mesh.h"

namespace xatlas
{
	inline glm::ivec2 BuildLightmapUV(std::vector<MyosotisFW::VertexData>& meshVertex, std::vector<uint32_t>& index)
	{
		ASSERT((!meshVertex.empty() && (!index.empty())), "Invalid mesh data for lightmap UV generation.");

		// xatlas のアトラス生成
		Atlas* atlas = Create();

		// メッシュ情報を xatlas 用に設定
		MeshDecl decl = {};
		decl.vertexCount = meshVertex.size();
		decl.vertexPositionData = &(meshVertex[0].position.x);		// 頂点位置バッファ先頭
		decl.vertexPositionStride = sizeof(MyosotisFW::VertexData); // 頂点1つのサイズ（バイト）
		decl.indexCount = index.size();
		decl.indexData = index.data();								// インデックスバッファ
		decl.indexFormat = IndexFormat::UInt32;

		// メッシュ登録（UV 展開の入力）
		AddMeshError::Enum addMeshError = AddMesh(atlas, decl);
		if (addMeshError != AddMeshError::Success)
		{
			Logger::Error("Failed to create lightmap.");
			Destroy(atlas);
			return glm::ivec2(0);
		}

		// UV 展開（チャート生成）とパッキングの設定
		ChartOptions chartOptions = {};
		PackOptions packOptions = {};

		packOptions.bilinear = true;       // 2×2 のブロック境界でアラインメント調整
		packOptions.blockAlign = true;     // ブロック境界にチャートを揃える
		packOptions.padding = 2;           // チャート間の余白（ピクセル単位）
		packOptions.texelsPerUnit = 32.0f; // 1m あたりのテクセル密度
		packOptions.resolution = 0;        // 0 = xatlas が最適解を自動決定

		// 実際に UV 展開とアトラス生成を実行
		Generate(atlas, chartOptions, packOptions);

		// 結果メッシュ（展開済み UV を保持）
		const Mesh& m = atlas->meshes[0];

		std::vector<MyosotisFW::VertexData> outVertex(m.vertexCount);
		std::vector<uint32_t> outIndex(m.indexCount);

		// vertex
		const float invW = 1.0f / static_cast<float>(atlas->width);
		const float invH = 1.0f / static_cast<float>(atlas->height);
		for (uint32_t v = 0; v < m.vertexCount; v++)
		{
			const Vertex& vertex = m.vertexArray[v];
			MyosotisFW::VertexData dst = meshVertex[vertex.xref];
			dst.uv1 = glm::vec2(vertex.uv[0] * invW, vertex.uv[1] * invH);
			outVertex[v] = dst;
		}

		// index
		for (uint32_t i = 0; i < m.indexCount; i++)
		{
			outIndex[i] = m.indexArray[i];
		}

		// データセット
		meshVertex = outVertex;
		index = outIndex;

		glm::ivec2 atlasSize = glm::ivec2(atlas->width, atlas->height);

		// 後始末
		Destroy(atlas);

		return atlasSize;
	}
}