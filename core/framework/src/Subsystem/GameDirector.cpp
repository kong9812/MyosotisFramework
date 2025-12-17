// Copyright (c) 2025 kong9812
#include "GameDirector.h"
#include "ComponentBase.h"
#include "RenderSubsystem.h"
#include "Logger.h"
#include "istduuid.h"
#include "CustomMesh.h"
#include "Terrain.h"
#include "GameStageIo.h"
#include "MObject.h"
#include "MObjectRegistry.h"

// TEST
#include "PrimitiveGeometry.h"
#include "FPSCamera.h"
// TEST

namespace MyosotisFW::System::GameDirector {
	GameDirector::GameDirector(const Render::RenderSubsystem_ptr& renderSubsystem) :
		m_renderSubsystem(renderSubsystem)
	{
		// TEST
		{
			MObject_ptr newObject = m_renderSubsystem->GetMObjectRegistry()->CreateNewObject();
			Render::Camera::FPSCamera_ptr component = Object_Cast<Render::Camera::FPSCamera>(
				System::ComponentFactory::CreateComponent(newObject->GetObjectID(), ComponentType::FPSCamera, newObject->GetMeshChangedCallback()));
			newObject->AddComponent(component);
			m_renderSubsystem->RegisterObject(newObject);
		}

		{
			MObject_ptr newObject = m_renderSubsystem->GetMObjectRegistry()->CreateNewObject();
			newObject->SetPos(glm::vec3(0.0f));
			newObject->SetRot(glm::vec3(0.0f));
			newObject->SetScale(glm::vec3(1.0f));

			Render::Terrain_ptr component = Object_Cast<Render::Terrain>(
				System::ComponentFactory::CreateComponent(newObject->GetObjectID(), ComponentType::Terrain, newObject->GetMeshChangedCallback()));
			Render::Terrain::MeshComponentInfo meshComponentInfo{};
			meshComponentInfo.terrainHeightmapName = "smallTerrain.png";
			component->SetMeshComponentInfo(meshComponentInfo);
			newObject->AddComponent(component);
			m_renderSubsystem->RegisterObject(newObject);
		}

		{
			MObject_ptr newObject = m_renderSubsystem->GetMObjectRegistry()->CreateNewObject();
			newObject->SetPos(glm::vec3(10.0f, 0.0f, 0.0f));
			newObject->SetRot(glm::vec3(0.0f, 180.0f, 0.0f));
			newObject->SetScale(glm::vec3(10.0f));

			Render::CustomMesh_ptr component = Object_Cast<Render::CustomMesh>(
				System::ComponentFactory::CreateComponent(newObject->GetObjectID(), ComponentType::CustomMesh, newObject->GetMeshChangedCallback()));
			Render::CustomMesh::MeshComponentInfo meshComponentInfo{};
			meshComponentInfo.meshName = "Suzanne.mfmodel";
			component->SetMeshComponentInfo(meshComponentInfo);
			newObject->AddComponent(component);
			m_renderSubsystem->RegisterObject(newObject);
		}

		{
			MObject_ptr newObject = m_renderSubsystem->GetMObjectRegistry()->CreateNewObject();
			newObject->SetPos(glm::vec3(-5.0f, 0.0f, 0.0f));
			newObject->SetRot(glm::vec3(0.0f, 180.0f, 0.0f));
			newObject->SetScale(glm::vec3(10.0f));

			Render::CustomMesh_ptr component = Object_Cast<Render::CustomMesh>(
				System::ComponentFactory::CreateComponent(newObject->GetObjectID(), ComponentType::CustomMesh, newObject->GetMeshChangedCallback()));
			Render::CustomMesh::MeshComponentInfo meshComponentInfo{};
			meshComponentInfo.meshName = "BarramundiFish/BarramundiFish.fbx";
			component->SetMeshComponentInfo(meshComponentInfo);
			newObject->AddComponent(component);
			m_renderSubsystem->RegisterObject(newObject);
		}

		{
			MObject_ptr newObject = m_renderSubsystem->GetMObjectRegistry()->CreateNewObject();
			newObject->SetPos(glm::vec3(30.0f, 0.0f, 0.0f));
			newObject->SetRot(glm::vec3(0.0f, 180.0f, 0.0f));
			newObject->SetScale(glm::vec3(10.0f));

			Render::CustomMesh_ptr component = Object_Cast<Render::CustomMesh>(
				System::ComponentFactory::CreateComponent(newObject->GetObjectID(), ComponentType::CustomMesh, newObject->GetMeshChangedCallback()));
			Render::CustomMesh::MeshComponentInfo meshComponentInfo{};
			meshComponentInfo.meshName = "Suzanne/Suzanne.gltf";
			component->SetMeshComponentInfo(meshComponentInfo);
			newObject->AddComponent(component);
			m_renderSubsystem->RegisterObject(newObject);
		}


		{
			MObject_ptr newObject = m_renderSubsystem->GetMObjectRegistry()->CreateNewObject();
			newObject->SetPos(glm::vec3(0.0f, 0.0f, -15.0f));
			newObject->SetRot(glm::vec3(0.0f));
			newObject->SetScale(glm::vec3(5.0f));

			Render::PrimitiveGeometry_ptr component = Object_Cast<Render::PrimitiveGeometry>(
				System::ComponentFactory::CreateComponent(newObject->GetObjectID(), ComponentType::PrimitiveGeometryMesh, newObject->GetMeshChangedCallback()));
			component->SetPrimitiveGeometryShape(Render::Shape::PrimitiveGeometryShape::Capsule);
			newObject->AddComponent(component);
			m_renderSubsystem->RegisterObject(newObject);
		}
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
