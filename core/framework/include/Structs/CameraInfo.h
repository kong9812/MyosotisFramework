// Copyright (c) 2025 kong9812
#pragma once
#include <vector>
#include "CameraData.h"

namespace MyosotisFW
{
	struct CameraInfo
	{
		uint32_t mainCameraIndex;
		std::vector<CameraData> cameraData;
	};
}
