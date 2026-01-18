// Copyright (c) 2025 kong9812
#pragma once
#include <string>
#include <filesystem>

namespace MyosotisFW
{
	struct FilePath
	{
		std::string path = "";

		std::filesystem::path GetFilesystemPath()
		{
			return std::filesystem::path(path);
		}

		bool empty()
		{
			return path.empty();
		}
	};
}
