// Copyright (c) 2025 kong9812
#pragma once
#include "MObjectRegistry.h"
#include "MObject.h"
#include "Logger.h"

#include "Camera.h"
#include "StaticMesh.h"

#include "RenderDescriptors.h"
#include "ComponentFactory.h"

// 実験
#include "Terrain.h"
// 実験

namespace MyosotisFW
{
	MObjectRegistry::MObjectRegistry(
		const System::Render::RenderDevice_ptr& device,
		const System::Render::RenderResources_ptr& resources,
		const System::Render::RenderDescriptors_ptr& descriptors) :
		m_device(device),
		m_resources(resources),
		m_renderDescriptors(descriptors),
		m_objects(std::make_shared<MObjectList>()),
		m_objectMap(),
		m_objectInfo(),
		m_meshChanged(true),
		m_transformChanged(true) {
	}

	MObject_ptr MObjectRegistry::CreateNewObject()
	{
		uint32_t objectIndex = static_cast<uint32_t>(m_objects->size());
		MObject_ptr newObject = m_objects->emplace_back(CreateMObjectPointer());
		m_objectMap.emplace(newObject->GetUUID(), newObject);	// 内部操作用
		ObjectInfo_ptr objectInfo = std::make_shared<ObjectInfo>(m_objectInfo.emplace_back());
		objectInfo->objectID = objectIndex;
		newObject->SetObjectInfo(objectInfo);

		newObject->SetMeshChangedCallback([this]() { this->MeshChanged(); });
		newObject->SetTransformChangedCallback([this]() { this->TransformChanged(); });

		m_onAddObject(newObject);

		return newObject;
	}

	ComponentBase_ptr MObjectRegistry::RegisterComponent(const uuids::uuid& uuid, const ComponentType type)
	{
		MObject_ptr object = m_objectMap[uuid];
		ComponentBase_ptr component = System::ComponentFactory::CreateComponent(object->GetObjectID(), type, object->GetMeshChangedCallback());
		object->AddComponent(component);

		switch (type)
		{
		case ComponentType::FPSCamera:
		{
			System::Render::Camera::CameraBase_ptr camera = System::Render::Camera::Object_CastToCameraBase(component);
			m_renderDescriptors->GetSceneInfoDescriptorSet()->AddCamera(camera);
		}
		break;
		case ComponentType::PrimitiveGeometryMesh:
		case ComponentType::CustomMesh:
		case ComponentType::Terrain:
		{
			System::Render::StaticMesh_ptr customMesh = System::Render::Object_CastToStaticMesh(component);
			customMesh->PrepareForRender(m_device, m_resources);
			m_meshChanged = true;
		}
		break;
		default:
			break;
		}

		// 実験
		if (type == ComponentType::Terrain)
		{
			System::Render::Terrain_ptr terrain = System::Render::Object_CastToTerrain(component);
			System::Render::Terrain::MeshComponentInfo meshComponentInfo{};
			meshComponentInfo.terrainHeightmapName.path = "smallTerrain.png";
			terrain->SetMeshComponentInfo(meshComponentInfo);
			m_meshChanged = true;
		}
		// 実験

		return component;
	}

	void MObjectRegistry::Clear()
	{
		m_objects->clear();
		m_objectMap.clear();
	}

	const ObjectInfo& MObjectRegistry::GetObjectInfo(const size_t index) const
	{
		ASSERT(m_objectInfo.size() > index, "ERROR!!! Out of bounds access.");
		return m_objectInfo[index];
	}

	std::vector<ObjectInfo> MObjectRegistry::GetAllObjectInfoCopy() const
	{
		std::vector<ObjectInfo> copy = { m_objectInfo.begin(), m_objectInfo.end() };
		return copy;
	}
};