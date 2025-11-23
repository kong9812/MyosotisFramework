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
	StaticMesh::StaticMesh(const uint32_t objectID) : ComponentBase(objectID),
		m_device(nullptr),
		m_resources(nullptr),
		m_vbDispatchInfo(),
		m_aabbMin(FLT_MAX),
		m_aabbMax(FLT_MIN)
	{
		m_name = "StaticMesh";
	}

	StaticMesh::~StaticMesh()
	{
	}

	void StaticMesh::PrepareForRender(const RenderDevice_ptr& device, const RenderResources_ptr& resources, const MeshInfoDescriptorSet_ptr& meshInfoDescriptorSet)
	{
		m_device = device;
		m_resources = resources;
		m_meshInfoDescriptorSet = meshInfoDescriptorSet;
	}

	void StaticMesh::Update(const UpdateData& updateData, const Camera::CameraBase_ptr& camera)
	{
		//if (camera)
		//{
		//	float distance = camera->GetDistance(m_transform.pos);
		//	if (distance <= m_lodDistances[LOD::LOD1])
		//	{
		//		m_currentLOD = LOD::LOD1;
		//	}
		//	else if (distance <= m_lodDistances[LOD::LOD2])
		//	{
		//		m_currentLOD = LOD::LOD2;
		//	}
		//	else if (distance <= m_lodDistances[LOD::LOD3])
		//	{
		//		m_currentLOD = LOD::LOD3;
		//	}
		//	else
		//	{
		//		//m_currentLOD = LOD::Hide;
		//	}
		//}
	}

	void StaticMesh::BindCommandBuffer(const VkCommandBuffer& commandBuffer)
	{
		/*if ((m_currentLOD == LOD::Hide) || (!m_isReady)) return;

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
		}*/
	}
}
