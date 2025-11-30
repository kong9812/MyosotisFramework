// Copyright (c) 2025 kong9812
#pragma once
#include <vector>
#include "CameraData.h"

namespace MyosotisFW
{
	struct CameraInfo
	{
		uint32_t mainCameraIndex;
		uint32_t _p1;
		uint32_t _p2;
		uint32_t _p3;
		std::vector<CameraData> cameraData;
	};
}
