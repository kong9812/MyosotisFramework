// Copyright (c) 2025 kong9812
#pragma once
#include "ClassPointer.h"
#include "ObjectType.h"
#include "Structs.h"
#include "istduuid.h"
#include "iRapidJson.h"
#include "istduuid.h"

namespace MyosotisFW 
{
	typedef struct
	{
		glm::vec3 pos;
		glm::vec3 rot;
		glm::vec3 scale;
	}Transform;

	class ObjectBase;
	TYPEDEF_SHARED_PTR_ARGS(ObjectBase)

	class ObjectBase
	{
	public:
		ObjectBase() :
			m_transfrom({ glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f) }),
			m_objectID(),
			m_isReady(false),
			m_name()
		{
			m_objectID = hashMaker();
			m_name = "object";
		};

		virtual ~ObjectBase() = default;

		// todo.コンポーネント関連の処理

		// Update
		//virtual void Update(const UpdateData& updateData) = 0;
		
		// Render
		//virtual void BindCommandBuffer(VkCommandBuffer commandBuffer) = 0;
		//// Debug
		//virtual void BindDebugGUIElement() = 0;

		virtual const ObjectType GetObjectType() const { return ObjectType::Undefined; }
		const std::string GetName() const { return m_name; }
		const uuids::uuid GetTypeID() const { return g_objectTypeUUID.at(GetObjectType()).value(); }
		const uuids::uuid GetObjectID() const { return m_objectID; }

		const glm::vec3 GetPos() const { return m_transfrom.pos; }
		const glm::vec3 GetRot() const { return m_transfrom.rot; }
		const glm::vec3 GetScale() const { return m_transfrom.scale; }
		const void SetPos(glm::vec3 pos) { m_transfrom.pos = pos; }
		const void SetRot(glm::vec3 rot) { m_transfrom.rot = rot; }
		const void SetScale(glm::vec3 scale) { m_transfrom.scale = scale; }

		// シリアルライズ
		virtual rapidjson::Value Serialize(rapidjson::Document::AllocatorType& allocator) const
		{
			rapidjson::Value obj(rapidjson::Type::kObjectType);
			obj.AddMember("id", rapidjson::Value(uuids::to_string(m_objectID).c_str(), allocator), allocator);
			obj.AddMember("name", rapidjson::Value(m_name.c_str(), allocator), allocator);
			obj.AddMember("typeID", rapidjson::Value(uuids::to_string(GetTypeID()).c_str(), allocator), allocator);

			SerializeVec3ToJson<glm::vec3>("pos", m_transfrom.pos, obj, allocator);
			SerializeVec3ToJson<glm::vec3>("rot", m_transfrom.rot, obj, allocator);
			SerializeVec3ToJson<glm::vec3>("scale", m_transfrom.scale, obj, allocator);

			// もし子要素があれば
			rapidjson::Value childrenArray(rapidjson::Type::kArrayType);
			for (const auto& child : m_childen)
			{
				childrenArray.PushBack(childrenArray.PushBack(child->Serialize(allocator), allocator), allocator);
			}
			obj.AddMember("children", childrenArray, allocator);

			return obj;
		}

		// デシリアルライズ
		virtual void Deserialize(const rapidjson::Value& doc, std::function<void(ObjectType, const rapidjson::Value&)> createObject)
		{
			m_objectID = uuids::uuid::from_string(doc["id"].GetString()).value();
			m_name = doc["name"].GetString();

			DeserializeVec3FromJson<glm::vec3>("pos", m_transfrom.pos, doc);
			DeserializeVec3FromJson<glm::vec3>("rot", m_transfrom.rot, doc);
			DeserializeVec3FromJson<glm::vec3>("scale", m_transfrom.scale, doc);

			// 子要素のデシリアル化
			if (doc.HasMember("children") && doc["children"].IsArray())
			{
				for (const auto& child : doc["children"].GetArray())
				{
					auto typeID = child["typeID"].GetString();
					auto optType = uuids::uuid::from_string(typeID);
					ObjectType type = findObjectTypeFromTypeID(optType.value());
					createObject(type, child);
				}
			}
		}

	protected:
		bool m_isReady;
		std::string m_name;
		uuids::uuid m_objectID;

		Transform m_transfrom;
		std::vector<ObjectBase_ptr> m_childen;
	};
};