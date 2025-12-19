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
	StaticMesh::StaticMesh(const uint32_t objectID, const std::function<void(void)>& meshChangedCallback) : ComponentBase(objectID),
		m_device(nullptr),
		m_resources(nullptr),
		m_vbDispatchInfo(),
		m_meshChangedCallback(meshChangedCallback),
		m_meshCount(0),
		m_meshID(0)
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
	}

	void StaticMesh::BindCommandBuffer(const VkCommandBuffer& commandBuffer)
	{
	}
}
