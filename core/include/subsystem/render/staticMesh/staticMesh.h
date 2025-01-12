// Copyright (c) 2025 kong9812
// Camera interface class
#pragma once
#include <vulkan/vulkan.h>

#include "classPointer.h"
#include "vkStruct.h"
#include "renderDevice.h"
#include "renderResources.h"

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
		StaticMesh(RenderDevice_ptr device, RenderResources_ptr resources, VkRenderPass renderPass, VkPipelineCache pipelineCache);
		~StaticMesh();

		virtual void BindCommandBuffer() {};

	protected:
		virtual void loadAssets() {};
		virtual void prepareUniformBuffers();
		virtual void prepareShaderStorageBuffers() {};

		// todo. descriptorsManager�Ɉڂ�
		// todo. descriptors��factory�ō��̂���������
		virtual void prepareDescriptors();

		// todo. renderpipeline��factory�ō��̂���������
		virtual void prepareRenderPipeline();

		// render device
		RenderDevice_ptr m_device;

		// render resources
		RenderResources_ptr m_resources;

		// render pass
		VkRenderPass m_renderPass;

		// pipeline cache
		VkPipelineCache m_pipelineCache;

		// pipeline
		VkPipeline m_pipeline;


		// todo.���̕ӂ�factory�ō��Ƃ�������
		VkDescriptorPool m_descriptorPool;
		VkDescriptorSet m_descriptorSet;

		VkDescriptorSetLayout m_descriptorSetLayout; // �p�C�v���C������鎞�ɕK�v�I�I

		// render pipeline
		VkPipelineLayout m_pipelineLayout;

		// ubo
		// todo. UBO�N���X��p�ӂ���(class StandardUBO)
		Utility::Vulkan::Struct::StaticMeshStandardUBO m_ubo;
		VkDescriptorBufferInfo m_uboDescriptor;
	};
	TYPEDEF_SHARED_PTR_ARGS(StaticMesh)
}
