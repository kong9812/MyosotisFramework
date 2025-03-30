// Copyright (c) 2025 kong9812
#include "FpsCamera.h"
#include "PrimitiveGeometry.h"
#include "CustomMesh.h"
#include "Skybox.h"
#include "InteriorObject.h"

namespace MyosotisFW::System::ObjectFactory
{
	inline ObjectBase_ptr CreateObject(const ObjectType& objectType)
	{
		ObjectBase_ptr object{};
		switch (objectType)
		{
		case ObjectType::FPSCamera:
		{
			object = Render::Camera::CreateFPSCameraPointer();
		}
		break;
		case ObjectType::PrimitiveGeometryMesh:
		{
			object = Render::CreatePrimitiveGeometryPointer();
		}
		break;
		case ObjectType::CustomMesh:
		{
			object = Render::CreateCustomMeshPointer();
		}
		break;
		case ObjectType::Skybox:
		{
			object = Render::CreateSkyboxPointer();
		}
		break;
		case ObjectType::InteriorObjectMesh:
		{
			object = Render::CreateInteriorObjectPointer();
		}
		default:
			break;
		}
		return object;
	}
}