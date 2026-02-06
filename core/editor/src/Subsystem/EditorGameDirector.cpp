// Copyright (c) 2025 kong9812
#include "EditorGameDirector.h"
#include "RenderSubsystem.h"
#include "MObject.h"
#include "MObjectRegistry.h"
#include "ComponentBase.h"
#include "MFWorldIo.h"

namespace MyosotisFW::System::GameDirector {
	void EditorGameDirector::LoadMFWorld(const std::string& fileName)
	{
		m_renderSubsystem->ResetWorld();

		std::vector<MObject_ptr> topObjects{};

		rapidjson::Document doc = Utility::Loader::loadMFWorld(fileName);
		if (doc.IsArray())
		{
			for (auto& obj : doc.GetArray())
			{
				if (obj.IsObject())
				{
					uuids::uuid uuid = uuids::uuid::from_string(obj["id"].GetString()).value();
					MObject_ptr newObject = m_renderSubsystem->GetMObjectRegistry()->CreateNewObject(&uuid);
					newObject->Deserialize(obj,
						[this](const uuids::uuid* uuid)
						{
							return m_renderSubsystem->GetMObjectRegistry()->CreateNewObject(uuid);
						},
						[this](const uuids::uuid& uuid, const ComponentType type)
						{
							return m_renderSubsystem->GetMObjectRegistry()->RegisterComponent(uuid, type);
						});
					topObjects.push_back(newObject);
				}
			}
		}

		m_mfWorldLoadedCallback(topObjects);
	}

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