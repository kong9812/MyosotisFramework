// Copyright (c) 2025 kong9812
#include "renderResources.h"
#include "vkLoader.h"

namespace MyosotisFW::System::Render
{
	RenderResources::RenderResources(RenderDevice_ptr device)
	{
		m_device = device;
	}

	RenderResources::~RenderResources()
	{
		for (std::pair<std::string, VkShaderModule> shaderMoudle : m_shaderModules)
		{
			vkDestroyShaderModule(*m_device, shaderMoudle.second, m_device->GetAllocationCallbacks());
		}
		m_shaderModules.clear();
	}

	VkShaderModule RenderResources::GetShaderModules(std::string fileName)
	{
		auto shaderModule = m_shaderModules.find(fileName);
		if (shaderModule == m_shaderModules.end())
		{
			// ないなら読み込む
			m_shaderModules.emplace(fileName, Utility::Loader::loadShader(*m_device, fileName, m_device->GetAllocationCallbacks()));
		}
		return m_shaderModules[fileName];
	}

	std::vector<Utility::Vulkan::Struct::Mesh> RenderResources::GetMeshVertex(std::string fileName)
	{
		auto vertexData = m_meshVertexDatas.find(fileName);
		if (vertexData == m_meshVertexDatas.end())
		{
			// ないなら読み込む
			m_meshVertexDatas.emplace(fileName, Utility::Loader::loadFbx(fileName));
		}
		return m_meshVertexDatas[fileName];
	}
}