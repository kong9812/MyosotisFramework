// Copyright (c) 2025 kong9812
#pragma once
#include <fstream>

#include "iRapidJson.h"
#include "AppInfo.h"
#include "Logger.h"

namespace Utility::Loader {
	inline rapidjson::Document loadMFWorld(std::string fileName)
	{
		std::ifstream file(fileName);
		ASSERT(file.is_open(), "Failed to open game stage file: " + fileName);
		rapidjson::IStreamWrapper istream(file);
		rapidjson::Document doc{};
		doc.ParseStream(istream);
		file.close();
		return doc;
	}
	inline void saveMFWorld(std::string fileName, rapidjson::Document& doc)
	{
		rapidjson::StringBuffer buffer{};
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
		doc.Accept(writer);
		std::ofstream file(fileName, std::ios::trunc);
		ASSERT(file.is_open(), "Failed to open game stage file: " + fileName);
		file << buffer.GetString();
		file.close();
	}
}
