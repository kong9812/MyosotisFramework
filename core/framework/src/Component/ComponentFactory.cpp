// Copyright (c) 2025 kong9812
#include "ComponentFactory.h"
#include "FpsCamera.h"
#include "PrimitiveGeometry.h"
#include "CustomMesh.h"
#include "Terrain.h"

namespace MyosotisFW::System::ComponentFactory
{
	ComponentBase_ptr CreateComponent(const uint32_t objectID, const ComponentType& objectType, const std::function<void(void)>& meshChangedCallback)
	{
		ComponentBase_ptr object{};
		switch (objectType)
		{
		case ComponentType::FPSCamera:
		{
			object = Render::Camera::CreateFPSCameraPointer(objectID);
		}
		break;
		case ComponentType::PrimitiveGeometryMesh:
		{
			object = Render::CreatePrimitiveGeometryPointer(objectID, meshChangedCallback);
		}
		break;
		case ComponentType::CustomMesh:
		{
			object = Render::CreateCustomMeshPointer(objectID, meshChangedCallback);
		}
		break;
		case ComponentType::Terrain:
		{
			object = Render::CreateTerrainPointer(objectID, meshChangedCallback);
		}
		break;
		default:
			break;
		}
		return object;
	}
}