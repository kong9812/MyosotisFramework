// Copyright (c) 2025 kong9812
#include "EditorGameDirector.h"
#include "RenderSubsystem.h"
#include "MObject.h"
#include "MObjectRegistry.h"
#include "ComponentBase.h"

namespace MyosotisFW::System::GameDirector {
	MObject_ptr EditorGameDirector::AddNewMObject()
	{
		MObject_ptr newObject = m_renderSubsystem->GetMObjectRegistry()->CreateNewObject();
		return newObject;
	}

	ComponentBase_ptr EditorGameDirector::RegisterComponent(const uuids::uuid& uuid, const MyosotisFW::ComponentType type)
	{
		return m_renderSubsystem->GetMObjectRegistry()->RegisterComponent(uuid, type);
	}
}