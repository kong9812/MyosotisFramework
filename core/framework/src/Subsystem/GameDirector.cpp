// Copyright (c) 2025 kong9812
#include "GameDirector.h"
#include "ComponentBase.h"
#include "RenderSubsystem.h"
#include "Logger.h"
#include "istduuid.h"
#include "CustomMesh.h"
#include "VK_Loader.h"
#include "MObject.h"
#include "MObjectRegistry.h"

// TEST
#include "PrimitiveGeometry.h"
#include "FPSCamera.h"
// TEST

namespace MyosotisFW::System::GameDirector {
	GameDirector::GameDirector(const Render::RenderSubsystem_ptr& renderSubsystem) :
		m_renderSubsystem(renderSubsystem),
		m_objectRegistry(CreateMObjectRegistryPointer())
	{
		// TEST
		{
			MObject_ptr newObject = m_objectRegistry->CreateNewObject();
			Render::Camera::FPSCamera_ptr component = Object_Cast<Render::Camera::FPSCamera>(
				System::ComponentFactory::CreateComponent(newObject->GetObjectID(), ComponentType::FPSCamera));
			newObject->AddComponent(component);
			m_renderSubsystem->RegisterObject(newObject);
		}

		{
			MObject_ptr newObject = m_objectRegistry->CreateNewObject();
			newObject->SetPos(glm::vec4(0.0f));
			newObject->SetRot(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
			newObject->SetScale(glm::vec4(5.0f));

			Render::PrimitiveGeometry_ptr component = Object_Cast<Render::PrimitiveGeometry>(
				System::ComponentFactory::CreateComponent(newObject->GetObjectID(), ComponentType::PrimitiveGeometryMesh));
			component->SetPrimitiveGeometryShape(Render::Shape::PrimitiveGeometryShape::Quad);
			newObject->AddComponent(component);
			m_renderSubsystem->RegisterObject(newObject);
		}
		//{
		//	MObject_ptr newObject = m_objectRegistry->CreateNewObject();
		//	newObject->SetPos(glm::vec4(0.0f));
		//	newObject->SetRot(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
		//	newObject->SetScale(glm::vec4(5.0f));

		//	Render::PrimitiveGeometry_ptr component = Object_Cast<Render::PrimitiveGeometry>(
		//		System::ComponentFactory::CreateComponent(newObject->GetObjectID(), ComponentType::PrimitiveGeometryMesh));
		//	component->SetPrimitiveGeometryShape(Render::Shape::PrimitiveGeometryShape::Plane);
		//	newObject->AddComponent(component);
		//	m_renderSubsystem->RegisterObject(newObject);
		//}

		//{
		//	MObject_ptr newObject = CreateMObjectPointer();
		//	Render::PrimitiveGeometry_ptr component = Object_Cast<Render::PrimitiveGeometry>(
		//		System::ComponentFactory::CreateComponent(ComponentType::PrimitiveGeometryMesh));
		//	component->SetPrimitiveGeometryShape(Render::Shape::PrimitiveGeometryShape::Quad);
		//	component->SetPos(glm::vec3(0.0f, 0.0f, 6.0f));
		//	component->SetRot(glm::vec3(0.0f, 0.0f, 0.0f));
		//	component->SetScale(glm::vec3(3.0f));
		//	newObject->AddComponent(component);
		//	m_renderSubsystem->RegisterObject(newObject);
		//}

		//// パフォーマンステスト用
		//const uint32_t objectCount = 10;
		//for (uint32_t x = 0; x < objectCount; x++)
		//{
		//	for (uint32_t z = 0; z < objectCount; z++)
		//	{
		//		{
		//			MObject_ptr newObject = CreateMObjectPointer();
		//			Render::CustomMesh_ptr component = Object_Cast<Render::CustomMesh>(
		//				System::ComponentFactory::CreateComponent(ComponentType::CustomMesh));
		//			CustomMeshInfo customMeshInfo{};
		//			customMeshInfo.meshName = "Suzanne/Suzanne.gltf";
		//			component->SetCustomMeshInfo(customMeshInfo);
		//			component->SetPos(glm::vec3(-20.0f + (5.0f * x), 0.0f, 50.0f + (5.0f * z)));
		//			component->SetRot(glm::vec3(0.0f, 0.0f, 0.0f));
		//			component->SetScale(glm::vec3(2.0f));
		//			newObject->AddComponent(component);
		//			m_renderSubsystem->RegisterObject(newObject);
		//		}
		//		{
		//			MObject_ptr newObject = CreateMObjectPointer();
		//			Render::CustomMesh_ptr component = Object_Cast<Render::CustomMesh>(
		//				System::ComponentFactory::CreateComponent(ComponentType::CustomMesh));
		//			CustomMeshInfo customMeshInfo{};
		//			customMeshInfo.meshName = "BarramundiFish/BarramundiFish.gltf";
		//			component->SetCustomMeshInfo(customMeshInfo);
		//			component->SetPos(glm::vec3(-20.0f + (5.0f * x), 5.0f, 50.0f + (5.0f * z)));
		//			component->SetRot(glm::vec3(0.0f, 0.0f, 0.0f));
		//			component->SetScale(glm::vec3(10.0f));
		//			newObject->AddComponent(component);
		//			m_renderSubsystem->RegisterObject(newObject);
		//		}
		//	}
		//}

		//// パフォーマンステスト用
		//const uint32_t objectCount = 10;
		//for (uint32_t x = 0; x < objectCount; x++)
		//{
		//	for (uint32_t z = 0; z < objectCount; z++)
		//	{
		//		for (uint32_t y = 0; y < objectCount; y++)
		//		{
		//			{
		//				MObject_ptr newObject = CreateMObjectPointer();
		//				newObject->SetPos(glm::vec3(2.0f * x, 2.0f * y, 15.0f + (2.0f * z)));
		//				newObject->SetRot(glm::vec3(0.0f, 0.0f, 0.0f));
		//				newObject->SetScale(glm::vec3(0.1f));
		//				Render::CustomMesh_ptr component = Object_Cast<Render::CustomMesh>(
		//					System::ComponentFactory::CreateComponent(ComponentType::CustomMesh));
		//				CustomMeshInfo customMeshInfo{};
		//				customMeshInfo.meshName = "Cube/Cube.gltf";
		//				component->SetCustomMeshInfo(customMeshInfo);
		//				newObject->AddComponent(component);
		//				m_renderSubsystem->RegisterObject(newObject);
		//			}
		//		}
		//	}
		//}

		//{
		//	MObject_ptr newObject = CreateMObjectPointer();
		//	newObject->SetPos(glm::vec3(0.0f, 0.0f, 10.0f));
		//	newObject->SetRot(glm::vec3(0.0f, 0.0f, 0.0f));
		//	newObject->SetScale(glm::vec3(3.0f, 3.0f, 0.1f));
		//	Render::CustomMesh_ptr component = Object_Cast<Render::CustomMesh>(
		//		System::ComponentFactory::CreateComponent(ComponentType::CustomMesh));
		//	CustomMeshInfo customMeshInfo{};
		//	customMeshInfo.meshName = "Cube/Cube.gltf";
		//	component->SetCustomMeshInfo(customMeshInfo);
		//	newObject->AddComponent(component);
		//	m_renderSubsystem->RegisterObject(newObject);
		//}

		//{
		//	MObject_ptr newObject = CreateMObjectPointer();
		//	Render::CustomMesh_ptr component = Object_Cast<Render::CustomMesh>(
		//		System::ComponentFactory::CreateComponent(ComponentType::CustomMesh));
		//	CustomMeshInfo customMeshInfo{};
		//	customMeshInfo.meshName = "Cube/Cube.gltf";
		//	component->SetCustomMeshInfo(customMeshInfo);
		//	component->SetPos(glm::vec3(0.0f, 0.0f, 15.0f));
		//	component->SetRot(glm::vec3(0.0f, 0.0f, 0.0f));
		//	component->SetScale(glm::vec3(1.0f));
		//	newObject->AddComponent(component);
		//	m_renderSubsystem->RegisterObject(newObject);
		//}

		//{
		//	MObject_ptr newObject = CreateMObjectPointer();
		//	Render::CustomMesh_ptr component = Object_Cast<Render::CustomMesh>(
		//		System::ComponentFactory::CreateComponent(ComponentType::CustomMesh));
		//	CustomMeshInfo customMeshInfo{};
		//	customMeshInfo.meshName = "Suzanne/Suzanne.gltf";
		//	component->SetCustomMeshInfo(customMeshInfo);
		//	component->SetPos(glm::vec3(-2.5f, 0.0f, 16.0f));
		//	component->SetRot(glm::vec3(0.0f, 0.0f, 0.0f));
		//	component->SetScale(glm::vec3(2.0f));
		//	newObject->AddComponent(component);
		//	m_renderSubsystem->RegisterObject(newObject);
		//}
		//{
		//	MObject_ptr newObject = CreateMObjectPointer();
		//	Render::CustomMesh_ptr component = Object_Cast<Render::CustomMesh>(
		//		System::ComponentFactory::CreateComponent(ComponentType::CustomMesh));
		//	CustomMeshInfo customMeshInfo{};
		//	customMeshInfo.meshName = "BarramundiFish/BarramundiFish.gltf";
		//	component->SetCustomMeshInfo(customMeshInfo);
		//	component->SetPos(glm::vec3(2.5f, 0.0f, 15.0f));
		//	component->SetRot(glm::vec3(0.0f, 0.0f, 0.0f));
		//	component->SetScale(glm::vec3(10.0f));
		//	newObject->AddComponent(component);
		//	m_renderSubsystem->RegisterObject(newObject);
		//}

		//{
		//	MObject_ptr newObject = CreateMObjectPointer();
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
		//	MObject_ptr newObject = CreateMObjectPointer();
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
					MObject_ptr newObject = CreateMObjectPointer();
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
