// Copyright (c) 2025 kong9812
#pragma once
#include <string>
#include <filesystem>

namespace MyosotisFW
{
	struct FilePath
	{
		std::string path = "";

		std::filesystem::path GetFilesystemPath() const
		{
			return std::filesystem::path(path);
		}

		const bool empty() const
		{
			return path.empty();
		}
	};
}
