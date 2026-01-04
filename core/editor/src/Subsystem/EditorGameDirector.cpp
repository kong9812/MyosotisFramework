// Copyright (c) 2025 kong9812
#include "EditorGameDirector.h"
#include "RenderSubsystem.h"
#include "MObject.h"
#include "MObjectRegistry.h"

namespace MyosotisFW::System::GameDirector {
	MObject_ptr EditorGameDirector::AddNewMObject()
	{
		MObject_ptr newObject = m_renderSubsystem->GetMObjectRegistry()->CreateNewObject();
		m_renderSubsystem->RegisterObject(newObject);
		return newObject;
	}
}
