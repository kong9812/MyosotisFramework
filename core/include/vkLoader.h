// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include <fstream>
#include <vector>

#include "iRapidJson.h"
#include "appInfo.h"
#include "logger.h"
#include "vkValidation.h"

namespace Utility::Vulkan::Loader
{
	inline VkShaderModule loadShader(VkDevice device, std::string fileName, const VkAllocationCallbacks* pAllocator = nullptr)
	{
		std::filesystem::path currentPath = std::filesystem::current_path();
		std::filesystem::path absolutePath = std::filesystem::absolute(MyosotisFW::AppInfo::g_shaderFolder + fileName);

		std::ifstream file(MyosotisFW::AppInfo::g_shaderFolder + fileName, std::ios::ate | std::ios::binary);
		ASSERT(file.is_open(), "Failed to open shader file: " + std::string(MyosotisFW::AppInfo::g_shaderFolder) + fileName);

		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buf(fileSize);
		file.seekg(0);
		file.read(buf.data(), fileSize);
		file.close();

		VkShaderModuleCreateInfo shaderModuleCreateInfo{};
		shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCreateInfo.codeSize = buf.size();
		shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(buf.data());

		VkShaderModule shaderModule{};
		VK_VALIDATION(vkCreateShaderModule(device, &shaderModuleCreateInfo, pAllocator, &shaderModule));
		return shaderModule;
	}

	/*inline rapidjson::Document loadGameStageFile(std::string fileName)
	{
		std::ifstream file(MyosotisFW::AppInfo::g_GameStageFolder + fileName);
		ASSERT(file.is_open(), "Failed to open shader file: " + std::string(MyosotisFW::AppInfo::g_GameStageFolder) + fileName);
		rapidjson::IStreamWrapper istream(file);
		rapidjson::Document doc{};
		doc.ParseStream(istream);
		file.close();
		return doc;
	}*/
}
