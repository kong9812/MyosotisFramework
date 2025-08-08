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
#include "InteriorObject.h"
#include "Skybox.h"
#include "FPSCamera.h"
// TEST

namespace MyosotisFW::System::GameDirector {
	GameDirector::GameDirector(const Render::RenderSubsystem_ptr& renderSubsystem) :
		m_renderSubsystem(renderSubsystem)
	{
		// TEST
		{
			StageObject_ptr newObject = CreateStageObjectPointer();
			Render::Camera::FPSCamera_ptr component = Object_Cast<Render::Camera::FPSCamera>(
				System::ComponentFactory::CreateComponent(ComponentType::FPSCamera));
			newObject->AddComponent(component);
			m_renderSubsystem->RegisterObject(newObject);
		}

		//{
		//	StageObject_ptr newObject = CreateStageObjectPointer();
		//	Render::Skybox_ptr component = Object_Cast<Render::Skybox>(
		//		System::ComponentFactory::CreateComponent(ComponentType::Skybox));
		//	component->SetPos(glm::vec3(0.0f));
		//	component->SetRot(glm::vec3(0.0f));
		//	component->SetScale(glm::vec3(1.0f));
		//	newObject->AddComponent(component);
		//	m_renderSubsystem->RegisterObject(newObject);
		//}

		//{
		//	StageObject_ptr newObject = CreateStageObjectPointer();
		//	Render::PrimitiveGeometry_ptr component = Object_Cast<Render::PrimitiveGeometry>(
		//		System::ComponentFactory::CreateComponent(ComponentType::PrimitiveGeometryMesh));
		//	component->SetPrimitiveGeometryShape(Render::Shape::PrimitiveGeometryShape::Quad);
		//	component->SetPos(glm::vec3(5.0f));
		//	component->SetRot(glm::vec3(40.0f, 0.0f, 0.0f));
		//	component->SetScale(glm::vec3(5.0f));
		//	newObject->AddComponent(component);
		//	m_renderSubsystem->RegisterObject(newObject);
		//}

		for (uint32_t i = 0; i < 10; i++)
		{
			for (uint32_t j = 0; j < 10; j++)
			{
				{
					StageObject_ptr newObject = CreateStageObjectPointer();
					Render::CustomMesh_ptr component = Object_Cast<Render::CustomMesh>(
						System::ComponentFactory::CreateComponent(ComponentType::CustomMesh));
					CustomMeshInfo customMeshInfo{};
					customMeshInfo.meshName = "Suzanne/Suzanne.gltf";
					component->SetCustomMeshInfo(customMeshInfo);
					component->SetPos(glm::vec3(-2.5f + (5.0f * j), 0.0f, 5.0f * i));
					component->SetRot(glm::vec3(0.0f, 0.0f, 0.0f));
					component->SetScale(glm::vec3(2.0f));
					newObject->AddComponent(component);
					m_renderSubsystem->RegisterObject(newObject);
				}
				{
					StageObject_ptr newObject = CreateStageObjectPointer();
					Render::CustomMesh_ptr component = Object_Cast<Render::CustomMesh>(
						System::ComponentFactory::CreateComponent(ComponentType::CustomMesh));
					CustomMeshInfo customMeshInfo{};
					customMeshInfo.meshName = "BarramundiFish/BarramundiFish.gltf";
					component->SetCustomMeshInfo(customMeshInfo);
					component->SetPos(glm::vec3(2.5f + (5.0f * j), 5.0f, 5.0f * i));
					component->SetRot(glm::vec3(0.0f, 0.0f, 0.0f));
					component->SetScale(glm::vec3(10.0f));
					newObject->AddComponent(component);
					m_renderSubsystem->RegisterObject(newObject);
				}
			}
		}

		//{
		//	StageObject_ptr newObject = CreateStageObjectPointer();
		//	Render::CustomMesh_ptr component = Object_Cast<Render::CustomMesh>(
		//		System::ComponentFactory::CreateComponent(ComponentType::CustomMesh));
		//	CustomMeshInfo customMeshInfo{};
		//	customMeshInfo.meshName = "Suzanne/Suzanne.gltf";
		//	component->SetCustomMeshInfo(customMeshInfo);
		//	component->SetPos(glm::vec3(-2.5f, 0.0f, 5.0f));
		//	component->SetRot(glm::vec3(0.0f, 0.0f, 0.0f));
		//	component->SetScale(glm::vec3(2.0f));
		//	newObject->AddComponent(component);
		//	m_renderSubsystem->RegisterObject(newObject);
		//}
		//{
		//	StageObject_ptr newObject = CreateStageObjectPointer();
		//	Render::CustomMesh_ptr component = Object_Cast<Render::CustomMesh>(
		//		System::ComponentFactory::CreateComponent(ComponentType::CustomMesh));
		//	CustomMeshInfo customMeshInfo{};
		//	customMeshInfo.meshName = "BarramundiFish/BarramundiFish.gltf";
		//	component->SetCustomMeshInfo(customMeshInfo);
		//	component->SetPos(glm::vec3(2.5f, 0.0f, 5.0f));
		//	component->SetRot(glm::vec3(0.0f, 0.0f, 0.0f));
		//	component->SetScale(glm::vec3(10.0f));
		//	newObject->AddComponent(component);
		//	m_renderSubsystem->RegisterObject(newObject);
		//}

		//{
		//	StageObject_ptr newObject = CreateStageObjectPointer();
		//	Render::PrimitiveGeometry_ptr component = Object_Cast<Render::PrimitiveGeometry>(
		//		System::ComponentFactory::CreateComponent(ComponentType::PrimitiveGeometryMesh));
		//	component->SetPrimitiveGeometryShape(Render::Shape::PrimitiveGeometryShape::Quad);
		//	component->SetPos(glm::vec3(-0.039652f, 0.077900f, -0.313738f));
		//	component->SetRot(glm::vec3(0.0f));
		//	component->SetScale(glm::vec3(0.1));
		//	newObject->AddComponent(component);
		//	m_renderSubsystem->RegisterObject(newObject);
		//}

		//{
		//	StageObject_ptr newObject = CreateStageObjectPointer();
		//	Render::InteriorObject_ptr component = Object_Cast<Render::InteriorObject>(
		//		System::ComponentFactory::CreateComponent(ComponentType::InteriorObjectMesh));
		//	component->SetPos(glm::vec3(5.0f));
		//	component->SetRot(glm::vec3(-90.0f, 0.0f, 0.0f));
		//	component->SetScale(glm::vec3(1.0f));
		//	newObject->AddComponent(component);
		//	m_renderSubsystem->RegisterObject(newObject);
		//}
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
