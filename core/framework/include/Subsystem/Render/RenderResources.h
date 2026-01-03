// Copyright (c) 2025 kong9812
#pragma once
#include <unordered_map>
#include <string>
#include <vulkan/vulkan.h>
#include <functional>
#include "Image.h"
#include "Mesh.h"
#include "ClassPointer.h"
#include "PrimitiveGeometryShape.h"
#include "BasicMaterial.h"
#include "AppInfo.h"

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
		RenderResources(const RenderDevice_ptr& device, const RenderDescriptors_ptr& renderDescriptors)
			:m_device(device),
			m_renderDescriptors(renderDescriptors),
			m_shaderModules({}),
			m_meshes({}),
			m_images({}),
			m_cubeImages({}),
			m_mainRenderTarget(),
			m_visibilityBuffer(),
			m_hiZDepthMap(),
			m_depthBuffer(),
			m_onLoadedMesh() {
		}
		~RenderResources();

		virtual void Initialize(const glm::ivec2& screenSize);

		VkShaderModule GetShaderModules(const std::string& fileName);
		MeshesHandle GetMesh(const std::string& fileName);
		MeshesHandle GetTerrainMesh(const std::string& fileName);
		MeshHandle GetPrimitiveGeometryMesh(const Shape::PrimitiveGeometryShape shape);
		Image GetImage(const std::string& fileName);
		Image GetCubeImage(const std::vector<std::string>& fileNames);
		virtual void Resize(const glm::ivec2& screenSize);
		VkSampler& CreateSampler(const VkSamplerCreateInfo& samplerCreateInfo);

		bool SaveImage(const Image& image, const std::string& fileName, const glm::ivec2& size);

		void SetOnLoadedMesh(const std::function<void(MeshesHandle&)>& callback) { m_onLoadedMesh = callback; }

		MeshHandle GetMeshFormID(const uint32_t meshID);

	protected:
		struct MeshData
		{
			Mesh_ptr mesh;
			MeshHandle meshHandle;
		};
		struct MeshesData
		{
			Meshes mesh;
			MeshesHandle meshHandle;
		};
		struct MaterialData
		{
			BasicMaterials material;
			BasicMaterialsHandle materialHandle;
		};

	protected:
		RenderDevice_ptr m_device;
		RenderDescriptors_ptr m_renderDescriptors;

	protected:
		// resources
		std::unordered_map<std::string, VkShaderModule> m_shaderModules;
		std::unordered_map<std::string, MeshesData> m_meshes;
		std::unordered_map<std::string, MaterialData> m_materials;
		std::unordered_map<Shape::PrimitiveGeometryShape, MeshData> m_primitiveGeometryMeshes;
		std::unordered_map<Shape::PrimitiveGeometryShape, MaterialData> m_primitiveGeometryMaterials;
		std::unordered_map<std::string, Image> m_images;
		std::unordered_map<std::string, Image> m_cubeImages;
		std::vector<VkSampler> m_samplers;

	public:
		// attachment
		Image& GetMainRenderTarget(const uint32_t index) { return m_mainRenderTarget[index]; }
		Image& GetVisibilityBuffer(const uint32_t index) { return m_visibilityBuffer[index]; }
		Image& GetLightmap(const uint32_t index) { return m_lightmap[index]; }
		Image& GetHiZDepthMap(const uint32_t index) { return m_hiZDepthMap[index]; }
		Image& GetDepthBuffer(const uint32_t index) { return m_depthBuffer[index]; }
		Image& GetRayTracingRenderTarget(const uint32_t index) { return m_rayTracingRenderTarget[index]; }

		Image& GetDummySampled2D() { return m_dummyImages.sampled2D; }
		Image& GetDummyStorage2D() { return m_dummyImages.storage2D; }

	protected:
		// attachment
		Image m_mainRenderTarget[AppInfo::g_maxInFlightFrameCount];
		Image m_rayTracingRenderTarget[AppInfo::g_maxInFlightFrameCount];
		Image m_visibilityBuffer[AppInfo::g_maxInFlightFrameCount];
		Image m_lightmap[AppInfo::g_maxInFlightFrameCount];
		Image m_hiZDepthMap[AppInfo::g_maxInFlightFrameCount];
		Image m_depthBuffer[AppInfo::g_maxInFlightFrameCount];

	protected:
		struct DummyImages
		{
			Image sampled2D;
			Image storage2D;
		}m_dummyImages;
		void destroyAllAttachment();
		void createVertexIndexBuffer(Meshes& meshes);
		void createAttachment(const glm::ivec2& screenSize);
		void createDefaultMaterial();
		void createDummyImages();

		std::function<void(MeshesHandle&)> m_onLoadedMesh;
	};
	TYPEDEF_SHARED_PTR_ARGS(RenderResources);
}