// Copyright (c) 2025 kong9812
#pragma once
#include <unordered_map>
#include <string>
#include <vulkan/vulkan.h>

#include "ClassPointer.h"
#include "RenderDevice.h"

namespace MyosotisFW::System::Render
{
	class RenderResources
	{
	public:
		RenderResources(RenderDevice_ptr device);
		~RenderResources();

		VkShaderModule GetShaderModules(std::string fileName);
		std::vector<Mesh> GetMeshVertex(std::string fileName);
		ImageWithSampler GetImage(std::string fileName);
		ImageWithSampler GetCubeImage(std::vector<std::string> fileNames);

	private:
		RenderDevice_ptr m_device;
		std::unordered_map<std::string, VkShaderModule> m_shaderModules;
		std::unordered_map<std::string, std::vector<Mesh>> m_meshVertexDatas;
		std::unordered_map<std::string, VMAImage> m_images;
		std::unordered_map<std::string, VMAImage> m_cubeImages;
	};
	TYPEDEF_SHARED_PTR_ARGS(RenderResources)
}