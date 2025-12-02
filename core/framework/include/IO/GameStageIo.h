// Copyright (c) 2025 kong9812
#pragma once
#include <fstream>

#include "iRapidJson.h"
#include "AppInfo.h"
#include "Logger.h"

namespace Utility::Loader {
	inline rapidjson::Document loadGameStageFile(std::string fileName)
	{
		std::ifstream file(MyosotisFW::AppInfo::g_gameStageFolder + fileName);
		ASSERT(file.is_open(), "Failed to open game stage file: " + std::string(MyosotisFW::AppInfo::g_gameStageFolder) + fileName);
		rapidjson::IStreamWrapper istream(file);
		rapidjson::Document doc{};
		doc.ParseStream(istream);
		file.close();
		return doc;
	}
	inline void saveGameStageFile(std::string fileName, rapidjson::Document& doc)
	{
		rapidjson::StringBuffer buffer{};
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		doc.Accept(writer);
		std::ofstream file(MyosotisFW::AppInfo::g_gameStageFolder + fileName, std::ios::trunc);
		ASSERT(file.is_open(), "Failed to open game stage file: " + std::string(MyosotisFW::AppInfo::g_gameStageFolder) + fileName);
		file << buffer.GetString();
		file.close();
	}
}
