// Copyright (c) 2025 kong9812
// For include xatlas
#pragma once
#include <xatlas.h>
#include <vector>
#include "Logger.h"

namespace xatlas
{
	inline void BuildLightmapUV(
		const float* positions, uint32_t vertexCount, uint32_t positionStride,
		const uint32_t* indices, uint32_t indexCount,
		std::vector<float>& outLmUV)
	{
		// xatlas のアトラス生成
		Atlas* atlas = Create();

		// メッシュ情報を xatlas 用に設定
		MeshDecl decl = {};
		decl.vertexCount = vertexCount;
		decl.vertexPositionData = positions;		// 頂点位置バッファ先頭
		decl.vertexPositionStride = positionStride; // 頂点1つのサイズ（バイト）
		decl.indexCount = indexCount;
		decl.indexData = indices;					// インデックスバッファ
		decl.indexFormat = IndexFormat::UInt32;

		// メッシュ登録（UV 展開の入力）
		AddMeshError::Enum addMeshError = AddMesh(atlas, decl);
		if (addMeshError != AddMeshError::Success)
		{
			Logger::Error("Failed to create lightmap.");
			Destroy(atlas);
			return;
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

		// 結果 UV 出力バッファを確保（頂点 × 2）
		outLmUV.resize(vertexCount * 2);

		// xatlas の UV はピクセル座標 → 0〜1 に正規化して格納
		for (uint32_t v = 0; v < vertexCount; v++)
		{
			const Vertex& vertex = m.vertexArray[v];
			float u = vertex.uv[0] / float(atlas->width);
			float vtex = vertex.uv[1] / float(atlas->height);

			outLmUV[v * 2 + 0] = u;
			outLmUV[v * 2 + 1] = vtex;
		}

		// 後始末
		Destroy(atlas);
	}
}