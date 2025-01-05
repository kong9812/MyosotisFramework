// Copyright (c) 2025 kong9812
// Camera interface class
#pragma once
#include <vulkan/vulkan.h>

#include "vkStruct.h"
#include "renderDevice.h"

namespace MyosotisFW::System::Render
{
	// todo.�t�@�N�g���[�p�^�[���ɂ���\��
	// StaticMeshFactory
	// �EPrimitiveGeometryMesh
	// �EFBXMesh
	// �E...
	class StaticMesh
	{
	public:
		//  todo.��������renderpipeline��descriptor���Ƃ��Ă���̂���������
		StaticMesh(RenderDevice_prt device);
		~StaticMesh();

	protected:
		// todo. descriptorsManager�Ɉڂ�
		// todo. descriptors��factory�ō��̂���������
		void prepareDescriptors();

		// todo. renderpipeline��factory�ō��̂���������
		void prepareRenderPipeline();

		// render device
		RenderDevice_prt m_device;

		// todo. UBO�N���X��p�ӂ���(class StandardUBO)
		Utility::Vulkan::Struct::StaticMeshStandardUBO m_ubo;

		// todo.���̕ӂ�factory�ō��Ƃ�������
		VkDescriptorPool m_descriptorPool;
		VkDescriptorSet m_descriptorSet;

		VkDescriptorSetLayout m_descriptorSetLayout; // �p�C�v���C������鎞�ɕK�v�I�I

		// render pipeline
		VkPipelineLayout m_pipelineLayout;

		// ubo
		VkDescriptorBufferInfo m_uboDescriptor;
	};
}
