// Copyright (c) 2025 kong9812
#include "StaticMesh.h"
#include <vector>

#include "RenderDevice.h"
#include "RenderResources.h"
#include "Camera.h"

#include "ivma.h"
#include "AppInfo.h"
#include "VK_CreateInfo.h"
#include "VK_Validation.h"

namespace MyosotisFW::System::Render
{
	StaticMesh::StaticMesh() : ComponentBase(),
		m_device(nullptr),
		m_resources(nullptr),
		m_vertexBuffer({}),
		m_indexBuffer({}),
		m_currentLOD(LOD::Hide),
		m_lodDistances({}),
		m_shadowMapShaderObject({}),
		m_staticMeshShaderObject({})
	{
		m_name = "StaticMesh";
		m_transform.scale = glm::vec3(1.0f);
		m_staticMeshShaderObject.normalMap.sampler = VK_NULL_HANDLE;
	}

	StaticMesh::~StaticMesh()
	{
		if (m_staticMeshShaderObject.normalMap.sampler)
		{
			vkDestroySampler(*m_device, m_staticMeshShaderObject.normalMap.sampler, m_device->GetAllocationCallbacks());
		}

		for (uint32_t logType = 0; logType < LOD::Max; logType++)
		{
			for (uint32_t meshIdx = 0; meshIdx < m_vertexBuffer[logType].size(); meshIdx++)
			{
				vmaDestroyBuffer(m_device->GetVmaAllocator(), m_vertexBuffer[logType][meshIdx].buffer, m_vertexBuffer[logType][meshIdx].allocation);
				vmaDestroyBuffer(m_device->GetVmaAllocator(), m_indexBuffer[logType][meshIdx].buffer, m_indexBuffer[logType][meshIdx].allocation);
			}
		}
	}

	void StaticMesh::PrepareForRender(const RenderDevice_ptr& device, const RenderResources_ptr& resources)
	{
		m_device = device;
		m_resources = resources;
		m_currentLOD = LOD::Hide;
		m_lodDistances = { AppInfo::g_defaultLODVeryClose, AppInfo::g_defaultLODClose, AppInfo::g_defaultLODFar };
	}

	void StaticMesh::Update(const UpdateData& updateData, const Camera::CameraBase_ptr& camera)
	{
		if (camera)
		{
			float distance = camera->GetDistance(m_transform.pos);
			if (distance <= m_lodDistances[LOD::LOD1])
			{
				m_currentLOD = LOD::LOD1;
			}
			else if (distance <= m_lodDistances[LOD::LOD2])
			{
				m_currentLOD = LOD::LOD2;
			}
			else if (distance <= m_lodDistances[LOD::LOD3])
			{
				m_currentLOD = LOD::LOD3;
			}
			else
			{
				//m_currentLOD = LOD::Hide;
			}
		}
		m_staticMeshShaderObject.SSBO.standardSSBO.model = glm::translate(glm::mat4(1.0f), glm::vec3(m_transform.pos));
		m_staticMeshShaderObject.SSBO.standardSSBO.model = glm::rotate(m_staticMeshShaderObject.SSBO.standardSSBO.model, glm::radians(m_transform.rot.x), glm::vec3(1.0f, 0.0f, 0.0f));
		m_staticMeshShaderObject.SSBO.standardSSBO.model = glm::rotate(m_staticMeshShaderObject.SSBO.standardSSBO.model, glm::radians(m_transform.rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
		m_staticMeshShaderObject.SSBO.standardSSBO.model = glm::rotate(m_staticMeshShaderObject.SSBO.standardSSBO.model, glm::radians(m_transform.rot.z), glm::vec3(0.0f, 0.0f, 1.0f));
		m_staticMeshShaderObject.SSBO.standardSSBO.model = glm::scale(m_staticMeshShaderObject.SSBO.standardSSBO.model, glm::vec3(m_transform.scale));
		m_staticMeshShaderObject.SSBO.standardSSBO.renderID = m_renderID;
		m_staticMeshShaderObject.SSBO.standardSSBO.position = glm::vec4(m_transform.pos, 0.0f);
		m_staticMeshShaderObject.SSBO.standardSSBO.rotation = glm::vec4(m_transform.rot, 0.0f);
		m_staticMeshShaderObject.SSBO.standardSSBO.scale = glm::vec4(m_transform.scale, 0.0f);
		m_shadowMapShaderObject.SSBO.standardSSBO = m_staticMeshShaderObject.SSBO.standardSSBO;
	}

	void StaticMesh::BindCommandBuffer(const VkCommandBuffer& commandBuffer, const RenderPipelineType& pipelineType)
	{
		if ((m_currentLOD == LOD::Hide) || (!m_isReady)) return;

		switch (pipelineType)
		{
		case RenderPipelineType::ShadowMap:
		{
			vkCmdBindDescriptorSets(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_shadowMapShaderObject.shaderBase.pipelineLayout, 0, 1, &m_shadowMapShaderObject.shaderBase.descriptorSet, 0, nullptr);
			vkCmdPushConstants(commandBuffer, m_shadowMapShaderObject.shaderBase.pipelineLayout,
				VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT,
				0,
				static_cast<uint32_t>(sizeof(m_shadowMapShaderObject.pushConstant)), &m_shadowMapShaderObject.pushConstant);
			vkCmdBindPipeline(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_shadowMapShaderObject.shaderBase.pipeline);
		}
		break;
		case RenderPipelineType::Deferred:
		{
			vkCmdBindDescriptorSets(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_staticMeshShaderObject.shaderBase.pipelineLayout, 0, 1, &m_staticMeshShaderObject.shaderBase.descriptorSet, 0, nullptr);
			vkCmdPushConstants(commandBuffer, m_staticMeshShaderObject.shaderBase.pipelineLayout,
				VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT,
				0,
				static_cast<uint32_t>(sizeof(m_staticMeshShaderObject.pushConstant)), &m_staticMeshShaderObject.pushConstant);
			vkCmdBindPipeline(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_staticMeshShaderObject.shaderBase.pipeline);
		}
		break;
		default:
			break;
		}

		const VkDeviceSize offsets[1] = { 0 };
		for (uint32_t meshIdx = 0; meshIdx < static_cast<uint32_t>(m_vertexBuffer[m_currentLOD].size()); meshIdx++)
		{
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuffer[m_currentLOD][meshIdx].buffer, offsets);
			vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer[m_currentLOD][meshIdx].buffer, 0, VkIndexType::VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer, m_indexBuffer[m_currentLOD][meshIdx].allocationInfo.size / sizeof(uint32_t), 1, 0, 0, 0);
		}
	}
}
