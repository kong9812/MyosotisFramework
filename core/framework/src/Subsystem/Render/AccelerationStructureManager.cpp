// Copyright (c) 2025 kong9812
#include "AccelerationStructureManager.h"

namespace MyosotisFW::System::Render
{
	void AccelerationStructureManager::OnLoadedMesh()
	{
		// 1. Vertex / Index Buffer 作成
		// 2. VkDeviceAddress 取得
		// 3. BLAS Geometry 情報構築
		// 4. BLAS 登録
		// 5. Dirty フラグ更新
		// 6. Bindless用の「登録要求」を生成
	}
}
