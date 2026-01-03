// Copyright (c) 2025 kong9812
#pragma once
#include <vector>
#include "CameraData.h"

namespace MyosotisFW
{
	struct CameraInfo
	{
		uint32_t mainCameraIndex = 0;
		uint32_t _p1 = 0;
		uint32_t _p2 = 0;
		uint32_t _p3 = 0;
		std::vector<CameraData> cameraData{};
	};
}
