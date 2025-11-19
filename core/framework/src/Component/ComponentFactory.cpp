// Copyright (c) 2025 kong9812
#include "ComponentFactory.h"
#include "FpsCamera.h"
#include "PrimitiveGeometry.h"
#include "CustomMesh.h"

namespace MyosotisFW::System::ComponentFactory
{
	ComponentBase_ptr CreateComponent(const ComponentType& objectType)
	{
		ComponentBase_ptr object{};
		switch (objectType)
		{
		case ComponentType::FPSCamera:
		{
			object = Render::Camera::CreateFPSCameraPointer();
		}
		break;
		case ComponentType::PrimitiveGeometryMesh:
		{
			object = Render::CreatePrimitiveGeometryPointer();
		}
		break;
		case ComponentType::CustomMesh:
		{
			object = Render::CreateCustomMeshPointer();
		}
		break;
		default:
			break;
		}
		return object;
	}
}