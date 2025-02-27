// Copyright (c) 2025 kong9812
#include "GameDirector.h"
#include "Logger.h"
#include "istduuid.h"
#include "CustomMesh.h"
#include "VK_Loader.h"

#include "ObjectFactory.h"

namespace {
	void CreateAndResistObject(
		MyosotisFW::System::GameDirector::GameDirector* gameDirector,
		MyosotisFW::System::Render::RenderSubsystem_ptr renderSubsystem,
		MyosotisFW::ObjectType type,
		const rapidjson::Value& doc) // docはconst参照で受け取る
	{
		// オブジェクトを生成
		MyosotisFW::ObjectBase_ptr obj = MyosotisFW::System::ObjectFactory::CreateObject(type);

		// オブジェクトをデシリアライズ
		obj->Deserialize(doc, [gameDirector, renderSubsystem](MyosotisFW::ObjectType childType, const rapidjson::Value& subDoc) {
			CreateAndResistObject(gameDirector, renderSubsystem, childType, subDoc);
			});

		// 生成したオブジェクトを登録
		renderSubsystem->RegisterObject(obj);
	}
}

namespace MyosotisFW::System::GameDirector {
	GameDirector::GameDirector(Render::RenderSubsystem_ptr& renderSubsystem)
	{
		m_renderSubsystem = renderSubsystem;

		LoadGameStageFile("TEST.gs");

		// TEST
		//MyosotisFW::ObjectBase_ptr obj = MyosotisFW::System::ObjectFactory::CreateObject(ObjectType::CustomMesh);
		//System::Render::CustomMesh_ptr customMesh = System::Render::Object_CastToCustomMesh(obj);
		//CustomMeshInfo customMeshInfo{};
		//customMeshInfo.m_meshPath = "test.fbx";
		//customMesh->SetCustomMeshInfo(customMeshInfo);
		//renderSubsystem->RegisterObject(obj);
		MyosotisFW::ObjectBase_ptr obj = MyosotisFW::System::ObjectFactory::CreateObject(ObjectType::Skybox);
		System::Render::Skybox_ptr skybox = System::Render::Object_CastToSkybox(obj);
		/*CustomMeshInfo customMeshInfo{};
		customMeshInfo.m_meshPath = "test.fbx";
		customMesh->SetCustomMeshInfo(customMeshInfo);*/
		renderSubsystem->RegisterObject(obj);
		// TEST
	}

	void GameDirector::LoadGameStageFile(std::string fileName)
	{
		// todo. load
		rapidjson::Document doc = Utility::Loader::loadGameStageFile(fileName);
		if (doc.IsArray())
		{
			for (auto& obj : doc.GetArray())
			{
				if (obj.IsObject())
				{
					auto typeID = obj["typeID"].GetString();
					auto optType = uuids::uuid::from_string(typeID);
					// todo.　もっと検証したい is〇〇を使う
					ObjectType type = findObjectTypeFromTypeID(optType.value());

					std::vector<ObjectBase_ptr> objects{};
					if (type == ObjectType::Undefined)
					{
						Logger::Error("Undefined Object Type!");
					}

					ObjectBase_ptr newObject = ObjectFactory::CreateObject(type);
					newObject->Deserialize(obj, [=](ObjectType type, const rapidjson::Value& subDoc) { CreateAndResistObject(this, m_renderSubsystem, type, subDoc); });
					m_renderSubsystem->RegisterObject(newObject);
				}
			}
		}
	}

	void GameDirector::SaveGameStageFile(std::string fileName, const std::vector<ObjectBase_ptr>& objects)
	{
		rapidjson::Document doc{};
		doc.SetArray();
		auto& allocator = doc.GetAllocator();
		for (const auto& object : objects)
		{
			doc.PushBack(object->Serialize(allocator), allocator);
		}

		Utility::Loader::saveGameStageFile(fileName, doc);
	}
}
