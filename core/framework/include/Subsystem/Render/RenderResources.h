// Copyright (c) 2025 kong9812
#pragma once
#include <unordered_map>
#include <string>
#include <vulkan/vulkan.h>
#include "Structs.h"
#include "Image.h"
#include "Mesh.h"
#include "ClassPointer.h"

namespace MyosotisFW::System::Render
{
	// 前方宣言
	class RenderDevice;
	TYPEDEF_SHARED_PTR_FWD(RenderDevice);

	class RenderResources
	{
	public:
		RenderResources(const RenderDevice_ptr& device)
			:m_device(device),
			m_shaderModules({}),
			m_meshVertexData({}),
			m_images({}),
			m_cubeImages({}),
			m_depthStencil({}),
			m_position({}),
			m_normal({}),
			m_baseColor({}),
			m_shadowMap({}),
			m_lightingResult({}),
			m_mainRenderTarget({}),
			m_visibilityBuffer({}),
			m_hiZDepthMap({}),
			m_primaryDepthStencil({}) {
		}
		~RenderResources();

		virtual void Initialize(const uint32_t width, const uint32_t height);

		VkShaderModule GetShaderModules(const std::string& fileName);
		std::vector<Mesh> GetMesh(const std::string& fileName);
		Image GetImage(const std::string& fileName);
		Image GetCubeImage(const std::vector<std::string>& fileNames);
		virtual void Resize(const uint32_t width, const uint32_t height);
		VkSampler& CreateSampler(const VkSamplerCreateInfo& samplerCreateInfo);

	protected:
		RenderDevice_ptr m_device;

		std::unordered_map<std::string, VkShaderModule> m_shaderModules;
		std::unordered_map<std::string, std::vector<Mesh>> m_meshVertexData;
		std::unordered_map<std::string, Image> m_images;
		std::unordered_map<std::string, Image> m_cubeImages;
		std::vector<VkSampler> m_samplers;

	public:
		Image& GetDepthStencil() { return m_depthStencil; }
		Image& GetPosition() { return m_position; }
		Image& GetNormal() { return m_normal; }
		Image& GetBaseColor() { return m_baseColor; }
		Image& GetShadowMap() { return m_shadowMap; }
		Image& GetLightingResult() { return m_lightingResult; }
		Image& GetMainRenderTarget() { return m_mainRenderTarget; }
		Image& GetVisibilityBuffer() { return m_visibilityBuffer; }

		Image& GetHiZDepthMap() { return m_hiZDepthMap; }
		Image& GetPrimaryDepthStencil() { return m_primaryDepthStencil; }

	protected:
		// attachments
		Image m_depthStencil;

		Image m_position;
		Image m_normal;
		Image m_baseColor;
		Image m_shadowMap;
		Image m_lightingResult;
		Image m_mainRenderTarget;
		Image m_visibilityBuffer;

		Image m_hiZDepthMap;
		Image m_primaryDepthStencil;
	};
	TYPEDEF_SHARED_PTR_ARGS(RenderResources);
}