// Copyright (c) 2025 kong9812
#include "GameDirector.h"
#include "ComponentBase.h"
#include "RenderSubsystem.h"
#include "Logger.h"
#include "istduuid.h"
#include "CustomMesh.h"
#include "VK_Loader.h"
#include "StageObject.h"

// TEST
#include "PrimitiveGeometry.h"
// TEST

namespace MyosotisFW::System::GameDirector {
	GameDirector::GameDirector(const Render::RenderSubsystem_ptr& renderSubsystem)
	{
		m_renderSubsystem = renderSubsystem;

		// TEST
		StageObject_ptr newObject = CreateStageObjectPointer();
		Render::PrimitiveGeometry_ptr component = Object_Cast<Render::PrimitiveGeometry>(
			System::ComponentFactory::CreateComponent(ComponentType::PrimitiveGeometryMesh));
		component->SetPos(glm::vec3(0.0f));
		component->SetPos(glm::vec3(0.0f));
		component->SetPos(glm::vec3(1.0f));
		newObject->AddComponent(component);
		m_renderSubsystem->RegisterObject(newObject);
		// TEST
	}

	void GameDirector::LoadGameStageFile(const std::string& fileName)
	{
		// object clear
		m_renderSubsystem->ResetGameStage();

		// todo. load
		rapidjson::Document doc = Utility::Loader::loadGameStageFile(fileName);
		if (doc.IsArray())
		{
			for (auto& obj : doc.GetArray())
			{
				if (obj.IsObject())
				{
					StageObject_ptr newObject = CreateStageObjectPointer();
					newObject->Deserialize(obj);
					m_renderSubsystem->RegisterObject(newObject);
				}
			}
		}
	}

	void GameDirector::SaveGameStageFile(const std::string& fileName, const std::vector<ComponentBase_ptr>& components)
	{
		rapidjson::Document doc{};
		doc.SetArray();
		auto& allocator = doc.GetAllocator();
		for (const auto& component : components)
		{
			doc.PushBack(component->Serialize(allocator), allocator);
		}

		Utility::Loader::saveGameStageFile(fileName, doc);
	}
}
