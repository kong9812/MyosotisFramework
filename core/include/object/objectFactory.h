// Copyright (c) 2025 kong9812
#include "fpsCamera.h"
#include "primitiveGeometry.h"
#include "customMesh.h"

namespace MyosotisFW::System::ObjectFactory
{
    inline ObjectBase_ptr CreateObject(ObjectType objectType)
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
		default:
			break;
		}
		return object;
    }
}