// Copyright (c) 2025 kong9812
#pragma once
#include <unordered_map>
#include <string>
#include <vulkan/vulkan.h>

#include "classPointer.h"
#include "renderDevice.h"

namespace MyosotisFW::System::Render
{
	class RenderResources
	{
	public:
		RenderResources(RenderDevice_ptr device);
		~RenderResources();

		VkShaderModule GetShaderModules(std::string fileName);

	private:
		RenderDevice_ptr m_device;
		std::unordered_map<std::string, VkShaderModule> m_shaderModules;
	};
	TYPEDEF_SHARED_PTR_ARGS(RenderResources)
}