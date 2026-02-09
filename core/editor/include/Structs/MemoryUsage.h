// Copyright (c) 2025 kong9812
#pragma once
#include <vector>

namespace MyosotisFW
{
	struct MemoryUsage
	{
		double totalSizeMB = 0.0;
		double totalUsageMB = 0.0;

		struct Detail
		{
			bool isDeviceLocal = false;
			double sizeMB = 0.0;
			double usageMB = 0.0;
		};
		std::vector<Detail> detail{};

		void Reset()
		{
			totalSizeMB = 0.0;
			totalUsageMB = 0.0;
			detail.clear();
		}
	};
}