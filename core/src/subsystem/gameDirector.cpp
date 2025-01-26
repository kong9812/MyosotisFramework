// Copyright (c) 2025 kong9812
#include "gameDirector.h"
#include "logger.h"
#include "istduuid.h"
#include "fpsCamera.h"
#include "primitiveGeometry.h"
#include "customMesh.h"
#include "vkLoader.h"

namespace {
	void CreateAndResistObject(
		MyosotisFW::System::GameDirector::GameDirector* gameDirector,
		MyosotisFW::System::Render::RenderSubsystem_ptr renderSubsystem,
		MyosotisFW::ObjectType type,
		const rapidjson::Value& doc) // docはconst参照で受け取る
	{
		// オブジェクトを生成
		MyosotisFW::ObjectBase_ptr obj = gameDirector->CreateObject(type);

		// オブジェクトをデシリアライズ
		obj->Deserialize(doc, [gameDirector, renderSubsystem](MyosotisFW::ObjectType childType, const rapidjson::Value& subDoc) {
			CreateAndResistObject(gameDirector, renderSubsystem, childType, subDoc);
			});

		// 生成したオブジェクトを登録
		renderSubsystem->ResistObject(obj);
	}
}

namespace MyosotisFW::System::GameDirector {
	GameDirector::GameDirector(Render::RenderSubsystem_ptr& renderSubsystem)
	{
		m_renderSubsystem = renderSubsystem;

		//// Test
		//ObjectBase_ptr staticMesh = CreateObject(ObjectType::StaticMesh);
		//m_renderSubsystem->ResistObject(staticMesh);

		//// Test
		//ObjectBase_ptr camera = CreateObject(ObjectType::Camera);
		//m_renderSubsystem->ResistObject(camera);
	}

	GameDirector::~GameDirector()
	{

	}

	ObjectBase_ptr GameDirector::CreateObject(ObjectType objectType)
	{
		ASSERT((objectType != ObjectType::Undefined) || (objectType != ObjectType::Max), "Error type");

		ObjectBase_ptr object{};

		// todo. factoryを用意
		switch (objectType)
		{
		case ObjectType::Camera:
		{
			object = Render::Camera::CreateFPSCameraPointer();
		}
		break;
		case ObjectType::StaticMesh:
		{
			object = Render::CreatePrimitiveGeometryPointer();

			//object = Render::CreatePrimitiveGeometryPointer();
		}
		break;
		default:
			break;
		}
		return object;
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
					if (type == ObjectType::Undefined)
					{
						Logger::Error("Undefined Object Type!");
					}
					switch (type)
					{
					case ObjectType::Camera:
					{
						ObjectBase_ptr newObject = Render::Camera::CreateFPSCameraPointer();
						newObject->Deserialize(obj, [=](ObjectType type, const rapidjson::Value& subDoc) { CreateAndResistObject(this, m_renderSubsystem, type, subDoc); });
						m_renderSubsystem->ResistObject(newObject);
					}
					break;
					case ObjectType::StaticMesh:
					{
						// todo.スタティックメッシュの種類
						ObjectBase_ptr newObject = Render::CreatePrimitiveGeometryPointer();
						newObject->Deserialize(obj, [=](ObjectType type, const rapidjson::Value& subDoc) { CreateAndResistObject(this, m_renderSubsystem, type, subDoc); });
						m_renderSubsystem->ResistObject(newObject);
					}
					break;
					default:
						break;
					}
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
