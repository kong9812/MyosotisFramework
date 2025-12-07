// Copyright (c) 2025 kong9812
#include "SceneInfoDescriptorSet.h"
#include "RenderDevice.h"
#include "Camera.h"

namespace MyosotisFW::System::Render
{
	SceneInfoDescriptorSet::SceneInfoDescriptorSet(const RenderDevice_ptr& device, const VkDescriptorPool& descriptorPool) :
		DescriptorSetBase(device, descriptorPool),
		m_cameras(),
		m_screenInfo(),
		m_cameraInfo(),
		m_sceneInfo(),
		m_terrainVBDispatchInfo()
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(DescriptorBindingIndex::Count); i++)
		{
			Descriptor descriptor{};
			descriptor.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptor.shaderStageFlagBits = VkShaderStageFlagBits::VK_SHADER_STAGE_ALL;
			descriptor.descriptorBindingFlags = VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
			descriptor.descriptorCount = 1;
			descriptor.rebuild = true;
			descriptor.update = true;
			m_descriptors.push_back(descriptor);
		}
		createDescriptorSet();
	}

	void SceneInfoDescriptorSet::Update()
	{
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::ScreenInfo)].rebuild)
		{
			uint32_t size = static_cast<uint32_t>(sizeof(ScreenInfo));
			buildDescriptor(static_cast<uint32_t>(DescriptorBindingIndex::ScreenInfo), size);
			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::ScreenInfo)].rebuild = false;
		}
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::SceneInfo)].rebuild)
		{
			uint32_t size = static_cast<uint32_t>(sizeof(SceneInfo));
			buildDescriptor(static_cast<uint32_t>(DescriptorBindingIndex::SceneInfo), size);
			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::SceneInfo)].rebuild = false;
		}
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::CameraInfo)].rebuild)
		{
			uint32_t size = (static_cast<uint32_t>(sizeof(uint32_t)) * 4) + (static_cast<uint32_t>(sizeof(CameraData)) * static_cast<uint32_t>(m_cameraInfo.cameraData.size()));
			buildDescriptor(static_cast<uint32_t>(DescriptorBindingIndex::CameraInfo), size);
			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::CameraInfo)].rebuild = false;
		}
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::TerrainVBDispatchInfo)].rebuild)
		{
			uint32_t size = static_cast<uint32_t>(sizeof(VBDispatchInfo)) * static_cast<uint32_t>(m_terrainVBDispatchInfo.size());
			buildDescriptor(static_cast<uint32_t>(DescriptorBindingIndex::TerrainVBDispatchInfo), size);
			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::TerrainVBDispatchInfo)].rebuild = false;
		}

		// SSBO/UBO更新
		updateScreenInfo();
		updateSceneInfo();
		updateCameraInfo();
		updateTerrainVBDispatchInfo();
	}

	void SceneInfoDescriptorSet::UpdateScreenSize(const glm::ivec2& screenSize)
	{
		m_screenInfo.screenSize = screenSize;
		m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::ScreenInfo)].update = true;
	}

	void SceneInfoDescriptorSet::AddCamera(const Camera::CameraBase_ptr& camera)
	{
		// Camerasに追加
		m_cameras.push_back(camera);
		// UpdateCameraInfoで更新
		m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::CameraInfo)].rebuild = true;
		// cameraDataサイズ
		m_cameraInfo.cameraData.resize(m_cameras.size());
	}

	void SceneInfoDescriptorSet::AddTerrainVBDispatchInfo(std::vector<VBDispatchInfo>& vbDispatchInfo)
	{
		m_terrainVBDispatchInfo.insert(m_terrainVBDispatchInfo.end(), vbDispatchInfo.begin(), vbDispatchInfo.end());
		m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::TerrainVBDispatchInfo)].update = true;
	}

	void SceneInfoDescriptorSet::updateScreenInfo()
	{
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::ScreenInfo)].update)
		{
			uint8_t* dst = static_cast<uint8_t*>(m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::ScreenInfo)].buffer.allocationInfo.pMappedData);
			memcpy(dst, &m_screenInfo, (static_cast<uint32_t>(sizeof(ScreenInfo))));
			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::ScreenInfo)].update = false;
		}
	}

	void SceneInfoDescriptorSet::updateSceneInfo()
	{
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::SceneInfo)].update)
		{
			uint8_t* dst = static_cast<uint8_t*>(m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::SceneInfo)].buffer.allocationInfo.pMappedData);
			memcpy(dst, &m_sceneInfo, (static_cast<uint32_t>(sizeof(SceneInfo))));
			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::SceneInfo)].update = false;
		}
	}

	void SceneInfoDescriptorSet::updateCameraInfo()
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(m_cameras.size()); i++)
		{
			const Camera::CameraBase_ptr& camera = m_cameras[i];
			if (camera->IsMainCamera())
			{
				m_cameraInfo.mainCameraIndex = i;
			}
			m_cameraInfo.cameraData[i] = camera->GetCameraData();
		}

		// 固定部分
		m_cameraInfo._p1 = 0;
		m_cameraInfo._p2 = 0;
		m_cameraInfo._p3 = 0;
		uint8_t* dst = static_cast<uint8_t*>(m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::CameraInfo)].buffer.allocationInfo.pMappedData);
		memcpy(dst, &m_cameraInfo.mainCameraIndex, (static_cast<uint32_t>(sizeof(uint32_t))));
		dst += static_cast<uint32_t>(sizeof(uint32_t));
		memcpy(dst, &m_cameraInfo._p1, (static_cast<uint32_t>(sizeof(uint32_t))));
		dst += static_cast<uint32_t>(sizeof(uint32_t));
		memcpy(dst, &m_cameraInfo._p2, (static_cast<uint32_t>(sizeof(uint32_t))));
		dst += static_cast<uint32_t>(sizeof(uint32_t));
		memcpy(dst, &m_cameraInfo._p3, (static_cast<uint32_t>(sizeof(uint32_t))));
		dst += static_cast<uint32_t>(sizeof(uint32_t));
		// 可変部分
		memcpy(dst, m_cameraInfo.cameraData.data(), sizeof(CameraData) * m_cameraInfo.cameraData.size());
	}

	void SceneInfoDescriptorSet::updateTerrainVBDispatchInfo()
	{
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::TerrainVBDispatchInfo)].update)
		{
			// 可変部分
			uint8_t* dst = static_cast<uint8_t*>(m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::TerrainVBDispatchInfo)].buffer.allocationInfo.pMappedData);
			memcpy(dst, m_terrainVBDispatchInfo.data(), sizeof(VBDispatchInfo) * m_terrainVBDispatchInfo.size());

			// 一時データクリア
			m_terrainVBDispatchInfo.clear();

			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::TerrainVBDispatchInfo)].update = false;
		}
	}
}