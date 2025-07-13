// Copyright (c) 2025 kong9812
#include "StaticMesh.h"
#include <vector>

#include "ivma.h"
#include "AppInfo.h"
#include "VK_CreateInfo.h"
#include "VK_Validation.h"

namespace MyosotisFW::System::Render
{
	StaticMesh::StaticMesh() : ObjectBase(), m_aabbMin(0.0f), m_aabbMax(0.0f)
	{
		m_name = "StaticMesh";
		m_transfrom.scale = glm::vec3(1.0f);
		m_staticMeshShaderObject = {};
		m_staticMeshShaderObject.standardUBO.normalMap.sampler = VK_NULL_HANDLE;
	}

	StaticMesh::~StaticMesh()
	{
		if (m_staticMeshShaderObject.standardUBO.normalMap.sampler)
		{
			vkDestroySampler(*m_device, m_staticMeshShaderObject.standardUBO.normalMap.sampler, m_device->GetAllocationCallbacks());
		}

		for (uint32_t logType = 0; logType < LOD::Max; logType++)
		{
			for (uint32_t meshIdx = 0; meshIdx < m_vertexBuffer[logType].size(); meshIdx++)
			{
				vmaDestroyBuffer(m_device->GetVmaAllocator(), m_vertexBuffer[logType][meshIdx].buffer, m_vertexBuffer[logType][meshIdx].allocation);
				vmaDestroyBuffer(m_device->GetVmaAllocator(), m_indexBuffer[logType][meshIdx].buffer, m_indexBuffer[logType][meshIdx].allocation);
			}
		}
		VK_VALIDATION(vkFreeDescriptorSets(*m_device, m_staticMeshShaderObject.deferredRenderShaderBase.descriptorPool, 1, &m_staticMeshShaderObject.deferredRenderShaderBase.descriptorSet));
		VK_VALIDATION(vkFreeDescriptorSets(*m_device, m_staticMeshShaderObject.shadowMapRenderShaderBase.descriptorPool, 1, &m_staticMeshShaderObject.shadowMapRenderShaderBase.descriptorSet));
		vmaDestroyBuffer(m_device->GetVmaAllocator(), m_staticMeshShaderObject.standardUBO.buffer.buffer, m_staticMeshShaderObject.standardUBO.buffer.allocation);
	}

	void StaticMesh::PrepareForRender(const RenderDevice_ptr& device, const RenderResources_ptr& resources)
	{
		m_device = device;
		m_resources = resources;
		m_currentLOD = LOD::Hide;
		m_lodDistances = { AppInfo::g_defaultLODVeryClose, AppInfo::g_defaultLODClose, AppInfo::g_defaultLODFar };

		vmaTools::ShaderBufferObjectAllocate(
			*m_device,
			m_device->GetVmaAllocator(),
			m_staticMeshShaderObject.standardUBO.data,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			m_staticMeshShaderObject.standardUBO.buffer.buffer,
			m_staticMeshShaderObject.standardUBO.buffer.allocation,
			m_staticMeshShaderObject.standardUBO.buffer.allocationInfo,
			m_staticMeshShaderObject.standardUBO.buffer.descriptor);
	}

	void StaticMesh::Update(const UpdateData& updateData, const Camera::CameraBase_ptr& camera)
	{
		if (camera)
		{
			float distance = camera->GetDistance(m_transfrom.pos);
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
		m_staticMeshShaderObject.standardUBO.data.renderID = m_renderID;
	}

	void StaticMesh::BindCommandBuffer(const VkCommandBuffer& commandBuffer, const RenderPipelineType& pipelineType)
	{
		if ((m_currentLOD == LOD::Hide) || (!m_isReady)) return;

		switch (pipelineType)
		{
		case RenderPipelineType::ShadowMap:
		{
			vkCmdBindDescriptorSets(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_staticMeshShaderObject.shadowMapRenderShaderBase.pipelineLayout, 0, 1, &m_staticMeshShaderObject.shadowMapRenderShaderBase.descriptorSet, 0, nullptr);
			vkCmdBindPipeline(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_staticMeshShaderObject.shadowMapRenderShaderBase.pipeline);
		}
		break;
		case RenderPipelineType::Deferred:
		{
			vkCmdBindDescriptorSets(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_staticMeshShaderObject.deferredRenderShaderBase.pipelineLayout, 0, 1, &m_staticMeshShaderObject.deferredRenderShaderBase.descriptorSet, 0, nullptr);
			vkCmdBindPipeline(commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_staticMeshShaderObject.deferredRenderShaderBase.pipeline);
		}
		break;
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

	OBBData StaticMesh::GetWorldOBBData()
	{
		OBBData obbData{};

		glm::vec3 localExtent = (m_aabbMax - m_aabbMin) * 0.5f;		// ローカルAABBの半径
		glm::vec3 centerLocal = (m_aabbMax + m_aabbMin) * 0.5f;		// ローカルAABBの中心
		glm::vec3 scaleExtent = localExtent * m_transfrom.scale;	// スケール適用

		// 回転を適用
		glm::mat3 rotMat = glm::mat3_cast(glm::quat(glm::radians(m_transfrom.rot)));
		obbData.axisX = glm::vec4(rotMat * glm::vec3(1.0f, 0.0f, 0.0f), scaleExtent.x);
		obbData.axisY = glm::vec4(rotMat * glm::vec3(0.0f, 1.0f, 0.0f), scaleExtent.y);
		obbData.axisZ = glm::vec4(rotMat * glm::vec3(0.0f, 0.0f, 1.0f), scaleExtent.z);

		// OBBの中心 (ワールド空間)
		obbData.center = glm::vec4(m_transfrom.pos + rotMat * (centerLocal * m_transfrom.scale), 0.0f);

		return obbData;
	}
}
