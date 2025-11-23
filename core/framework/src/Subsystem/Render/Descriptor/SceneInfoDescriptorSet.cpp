// Copyright (c) 2025 kong9812
#include "SceneInfoDescriptorSet.h"
#include "RenderDevice.h"
#include "Camera.h"

namespace MyosotisFW::System::Render
{
	SceneInfoDescriptorSet::SceneInfoDescriptorSet(const RenderDevice_ptr& device, const VkDescriptorPool& descriptorPool) :
		DescriptorSetBase(device, descriptorPool),
		m_cameras(),
		m_cameraInfo()
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(DescriptorBindingIndex::Count); i++)
		{
			Descriptor descriptor{};
			descriptor.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptor.shaderStageFlagBits = VkShaderStageFlagBits::VK_SHADER_STAGE_ALL;
			descriptor.descriptorBindingFlags = VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
			descriptor.rebuild = true;
			descriptor.update = true;
			m_descriptors.push_back(descriptor);
		}
	}

	void SceneInfoDescriptorSet::Update()
	{
		if (m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::CameraInfo)].rebuild)
		{
			uint32_t size = static_cast<uint32_t>(sizeof(uint32_t)) + (static_cast<uint32_t>(sizeof(CameraData)) * static_cast<uint32_t>(m_cameraInfo.cameraData.size()));
			buildDescriptor(static_cast<uint32_t>(DescriptorBindingIndex::CameraInfo), size);
			m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::CameraInfo)].rebuild = false;
		}

		// SSBO/UBO更新
		updateCameraInfo();
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

	void SceneInfoDescriptorSet::updateCameraInfo()
	{
		for (uint32_t i = 0; static_cast<uint32_t>(m_cameras.size()); i++)
		{
			const Camera::CameraBase_ptr& camera = m_cameras[i];
			if (camera->IsMainCamera())
			{
				m_cameraInfo.mainCameraIndex = i;
			}
			m_cameraInfo.cameraData[i] = camera->GetCameraData();
		}

		// 固定部分
		uint8_t* dst = static_cast<uint8_t*>(m_descriptors[static_cast<uint32_t>(DescriptorBindingIndex::CameraInfo)].buffer.allocationInfo.pMappedData);
		memcpy(dst, &m_cameraInfo.mainCameraIndex, sizeof(m_cameraInfo.mainCameraIndex));
		dst += static_cast<uint32_t>(sizeof(m_cameraInfo.mainCameraIndex));
		// 可変部分
		memcpy(dst, m_cameraInfo.cameraData.data(), sizeof(CameraData) * m_cameraInfo.cameraData.size());
	}
}