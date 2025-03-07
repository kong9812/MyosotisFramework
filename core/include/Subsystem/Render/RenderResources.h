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
		RenderResources(const RenderDevice_ptr& device, const uint32_t width, const uint32_t height);
		~RenderResources();

		VkShaderModule GetShaderModules(const std::string& fileName);
		std::vector<Mesh> GetMeshVertex(const std::string& fileName);
		ImageWithSampler GetImage(const std::string& fileName);
		ImageWithSampler GetCubeImage(const std::vector<std::string>& fileNames);

	private:
		RenderDevice_ptr m_device;
		std::unordered_map<std::string, VkShaderModule> m_shaderModules;
		std::unordered_map<std::string, std::vector<Mesh>> m_meshVertexDatas;
		std::unordered_map<std::string, VMAImage> m_images;
		std::unordered_map<std::string, VMAImage> m_cubeImages;

	public:
		void prepareAttachments(const uint32_t width, const uint32_t height);

		VMAImage& GetPosition() { return m_position; }
		VMAImage& GetNormal() { return m_normal; }
		VMAImage& GetBaseColor() { return m_baseColor; }
		VMAImage& GetShadowMap() { return m_shadowMap; }

	private:
		// attachments
		VMAImage m_position;
		VMAImage m_normal;
		VMAImage m_baseColor;
		VMAImage m_shadowMap;

	};
	TYPEDEF_SHARED_PTR_ARGS(RenderResources)
}