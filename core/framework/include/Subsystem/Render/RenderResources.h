// Copyright (c) 2025 kong9812
#pragma once
#include <unordered_map>
#include <string>
#include <vulkan/vulkan.h>
#include <functional>
#include "Structs.h"
#include "Image.h"
#include "Mesh.h"
#include "ClassPointer.h"
#include "PrimitiveGeometryShape.h"

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
			m_meshes({}),
			m_images({}),
			m_cubeImages({}),
			m_mainRenderTarget({}),
			m_visibilityBuffer({}),
			m_hiZDepthMap({}),
			m_primaryDepthStencil({}),
			m_onLoadedMesh() {
		}
		~RenderResources();

		virtual void Initialize(const uint32_t width, const uint32_t height);

		VkShaderModule GetShaderModules(const std::string& fileName);
		std::vector<Mesh> GetMesh(const std::string& fileName);
		Mesh GetPrimitiveGeometryMesh(const Shape::PrimitiveGeometryShape shape);
		Image GetImage(const std::string& fileName);
		Image GetCubeImage(const std::vector<std::string>& fileNames);
		virtual void Resize(const uint32_t width, const uint32_t height);
		VkSampler& CreateSampler(const VkSamplerCreateInfo& samplerCreateInfo);

		bool SaveImage(const Image& image, const std::string& fileName, const glm::ivec2& size);

		void SetOnLoadedMesh(const std::function<void(const std::vector<Mesh>&)>& callback) { m_onLoadedMesh = callback; }

	protected:
		RenderDevice_ptr m_device;

		std::unordered_map<std::string, VkShaderModule> m_shaderModules;
		std::unordered_map<std::string, std::vector<Mesh>> m_meshes;
		std::unordered_map<Shape::PrimitiveGeometryShape, Mesh> m_primitiveGeometryMeshes;
		std::unordered_map<std::string, Image> m_images;
		std::unordered_map<std::string, Image> m_cubeImages;
		std::vector<VkSampler> m_samplers;

	public:
		Image& GetMainRenderTarget() { return m_mainRenderTarget; }
		Image& GetVisibilityBuffer() { return m_visibilityBuffer; }
		Image& GetLightmap() { return m_lightmap; }

		Image& GetHiZDepthMap() { return m_hiZDepthMap; }
		Image& GetPrimaryDepthStencil() { return m_primaryDepthStencil; }

		Image& GetRayTracingRenderTarget() { return m_rayTracingRenderTarget; }

	protected:
		Image m_mainRenderTarget;
		Image m_visibilityBuffer;
		Image m_lightmap;
		Image m_rayTracingRenderTarget;

		Image m_hiZDepthMap;
		Image m_primaryDepthStencil;

	protected:
		void createVertexIndexBuffer(std::vector<Mesh>& meshes);

	protected:
		std::function<void(const std::vector<Mesh>&)> m_onLoadedMesh;
	};
	TYPEDEF_SHARED_PTR_ARGS(RenderResources);
}