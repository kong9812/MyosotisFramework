// Copyright (c) 2025 kong9812
#include "RenderResources.h"
#include "VK_Loader.h"

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

		m_meshVertexDatas.clear();

		for (std::pair<std::string, VMAImage> image : m_images)
		{
			vmaDestroyImage(m_device->GetVmaAllocator(), image.second.image, image.second.allocation);
			vkDestroyImageView(*m_device, image.second.view, m_device->GetAllocationCallbacks());
		}
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

	std::vector<Mesh> RenderResources::GetMeshVertex(std::string fileName)
	{
		auto vertexData = m_meshVertexDatas.find(fileName);
		if (vertexData == m_meshVertexDatas.end())
		{
			// ないなら読み込む
			m_meshVertexDatas.emplace(fileName, Utility::Loader::loadFbx(fileName));
		}
		return m_meshVertexDatas[fileName];
	}

	ImageWithSampler RenderResources::GetImage(std::string fileName)
	{
		auto image = m_images.find(fileName);
		if (image == m_images.end())
		{
			// todo.　実装場所を変える
			VkCommandPoolCreateInfo commandPoolCreateInfo = Utility::Vulkan::CreateInfo::commandPoolCreateInfo(m_device->GetTransferFamilyIndex());
			VkCommandPool pool{};
			VkQueue queue{};
			vkGetDeviceQueue(*m_device, m_device->GetTransferFamilyIndex(), 0, &queue);
			VK_VALIDATION(vkCreateCommandPool(*m_device, &commandPoolCreateInfo, m_device->GetAllocationCallbacks(), &pool));

			// ないなら読み込む
			m_images.emplace(fileName, Utility::Loader::loadImage(
				*m_device,
				queue,
				pool,
				m_device->GetVmaAllocator(),
				fileName,
				m_device->GetAllocationCallbacks()));

			// 後片付け
			vkDestroyCommandPool(*m_device, pool, m_device->GetAllocationCallbacks());
		}
		return { m_images[fileName].image,  m_images[fileName].view };
	}
}