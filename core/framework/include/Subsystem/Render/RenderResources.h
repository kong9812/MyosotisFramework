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
		RenderResources(const RenderDevice_ptr& device)
			:m_device(device),
			m_depthStencil{},
			m_position{},
			m_normal{},
			m_baseColor{},
			m_shadowMap{},
			m_lightingResult{},
			m_mainRenderTarget{} {
		}
		~RenderResources();

		virtual void Initialize(const uint32_t width, const uint32_t height);

		VkShaderModule GetShaderModules(const std::string& fileName);
		std::vector<Mesh> GetMeshVertex(const std::string& fileName);
		Image GetImage(const std::string& fileName);
		Image GetCubeImage(const std::vector<std::string>& fileNames);
		virtual void Resize(const uint32_t width, const uint32_t height);

	protected:
		RenderDevice_ptr m_device;
		std::unordered_map<std::string, VkShaderModule> m_shaderModules;
		std::unordered_map<std::string, std::vector<Mesh>> m_meshVertexDatas;
		std::unordered_map<std::string, VMAImage> m_images;
		std::unordered_map<std::string, VMAImage> m_cubeImages;

	public:
		DeviceImage& GetDepthStencil() { return m_depthStencil; }
		VMAImage& GetPosition() { return m_position; }
		VMAImage& GetNormal() { return m_normal; }
		VMAImage& GetBaseColor() { return m_baseColor; }
		VMAImage& GetShadowMap() { return m_shadowMap; }
		VMAImage& GetLightingResult() { return m_lightingResult; }
		VMAImage& GetMainRenderTarget() { return m_mainRenderTarget; }
		VMAImage& GetIdMap() { return m_idMap; }

	protected:
		// attachments
		DeviceImage m_depthStencil;

		VMAImage m_position;
		VMAImage m_normal;
		VMAImage m_baseColor;
		VMAImage m_shadowMap;
		VMAImage m_lightingResult;
		VMAImage m_mainRenderTarget;
		VMAImage m_idMap;
	};
	TYPEDEF_SHARED_PTR_ARGS(RenderResources)
}