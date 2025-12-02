// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include <fstream>
#include <vector>
#include <chrono>

#include "istb_image.h"
#include "AppInfo.h"

#include "Logger.h"
#include "Image.h"
#include "VK_Validation.h"

namespace Utility::Loader {
	inline VkShaderModule loadShader(VkDevice device, std::string fileName, const VkAllocationCallbacks* pAllocator = nullptr)
	{
#ifdef DEBUG
		Logger::Debug("[VK_Loader] Start load: " + fileName);
		auto start = std::chrono::high_resolution_clock::now();
#endif
		std::filesystem::path absolutePath = std::filesystem::absolute(MyosotisFW::AppInfo::g_shaderFolder + fileName);

		std::ifstream file(absolutePath.string().c_str(), std::ios::ate | std::ios::binary);
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
#ifdef DEBUG
		Logger::Debug("[VK_Loader] End load: " + fileName +
			"(" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count()) + "ms)");
#endif
		return shaderModule;
	}
}
