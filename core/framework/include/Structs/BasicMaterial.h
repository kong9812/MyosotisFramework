// Copyright (c) 2025 kong9812
#pragma once
#include <vector>
#include <string>
#include "iglm.h"
#include "ClassPointer.h"
#include "BasicMaterialInfo.h"

namespace MyosotisFW
{
	struct BasicMaterial
	{
		// GPU Data
		BasicMaterialInfo basicMaterialInfo;

		// Image
		std::string baseColorTexturePath;
		std::string normalTexturePath;
	};
	TYPEDEF_SHARED_PTR(BasicMaterial);								// RenderResoureces保持用
	using BasicMaterialHandle = std::weak_ptr<const BasicMaterial>;	// 外部参照用

	// 複数メッシュ管理
	using BasicMaterials = std::vector<BasicMaterial_ptr>;			// RenderResoureces保持用
	using BasicMaterialsHandle = std::vector<BasicMaterialHandle>;	// 外部参照用
}
