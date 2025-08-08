// Copyright (c) 2025 kong9812
#pragma once
#include <unordered_map>
#include <string>
#include <vulkan/vulkan.h>
#include "Structs.h"
#include "ClassPointer.h"

namespace MyosotisFW::System::Render
{
	// 前方宣言
	class RenderDevice;
	TYPEDEF_SHARED_PTR_FWD(RenderDevice);
	class RenderDescriptors;
	TYPEDEF_SHARED_PTR_FWD(RenderDescriptors);

	class RenderResources
	{
	public:
		RenderResources(const RenderDevice_ptr& device, const RenderDescriptors_ptr& descriptors)
			:m_device(device),
			m_descriptors(descriptors),
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
			m_idMap({}),
			m_hiZDepthMap({}),
			m_primaryDepthStencil({}) {
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
		RenderDescriptors_ptr m_descriptors;

		std::unordered_map<std::string, VkShaderModule> m_shaderModules;
		std::unordered_map<std::string, std::vector<Mesh>> m_meshVertexData;
		std::unordered_map<std::string, VMAImage> m_images;
		std::unordered_map<std::string, VMAImage> m_cubeImages;
		std::unordered_map<std::string, uint32_t> m_meshID;

	public:
		DeviceImage& GetDepthStencil() { return m_depthStencil; }
		VMAImage& GetPosition() { return m_position; }
		VMAImage& GetNormal() { return m_normal; }
		VMAImage& GetBaseColor() { return m_baseColor; }
		VMAImage& GetShadowMap() { return m_shadowMap; }
		VMAImage& GetLightingResult() { return m_lightingResult; }
		VMAImage& GetMainRenderTarget() { return m_mainRenderTarget; }
		VMAImage& GetIdMap() { return m_idMap; }
		uint32_t& GetMeshID(const std::string& fileName);

		VMAMipImage& GetHiZDepthMap() { return m_hiZDepthMap; }
		VMAImage& GetPrimaryDepthStencil() { return m_primaryDepthStencil; }

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

		VMAMipImage m_hiZDepthMap;
		VMAImage m_primaryDepthStencil;
	};
	TYPEDEF_SHARED_PTR_ARGS(RenderResources);
}