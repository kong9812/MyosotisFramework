// Copyright (c) 2025 kong9812
#pragma once
#include <unordered_map>
#include "ClassPointer.h"
#include "AccelerationStructure.h"

namespace MyosotisFW::System::Render
{
	class AccelerationStructureManager
	{
	public:
		AccelerationStructureManager() :
			m_blasDirty(false),
			m_tlasDirty(false),
			m_blas({}),
			m_tlas({}) {
		}
		~AccelerationStructureManager() {}

		void OnLoadedMesh();
		// todo. Meshに対応するBLASを登録し、ビルドが必要な状態にする
		// todo. MeshデータからBLASを生成するためのリソースを作成し、
		// todo. Vertex/IndexのデバイスアドレスとBLASを登録する

		void OnAddObject() {}		// todo. TLAS用のInstance情報を追加し、TLAS更新を要求する

		void NewScene() {}			// todo. シーン切り替え時にTLASとInstance情報をリセットする

		void Process() {}			//todo. ここでビルドと更新を判定し、実行する

	private:
		bool m_blasDirty;
		bool m_tlasDirty;

		std::unordered_map<uint32_t, AccelerationStructure> m_blas;
		AccelerationStructure m_tlas;

		void buildBLAS() {}	// todo. 登録済みBLASをGPU上にBuildする（初回のみ）
		void buildTLAS() {} // todo. TLASのビルド処理

		void updateTLAS() {} // todo. TLASの更新処理

		// まぁやること多い…！でもレイトレーシングの結果を早く見たい！！！
	};
	TYPEDEF_SHARED_PTR_ARGS(AccelerationStructureManager);
}