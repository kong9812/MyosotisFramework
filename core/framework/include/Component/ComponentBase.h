// Copyright (c) 2025 kong9812
#pragma once
#include "ClassPointer.h"
#include "ComponentType.h"
#include "Transform.h"
#include "Structs.h"
#include "istduuid.h"
#include "iRapidJson.h"
#include "ComponentFactory.h"

namespace MyosotisFW
{
	class ComponentBase;
	TYPEDEF_SHARED_PTR(ComponentBase);

	class ComponentBase
	{
	public:
		ComponentBase() :
			m_objectID(),
			m_isReady(false),
			m_name(),
			m_renderID(0)
		{
			m_objectID = hashMaker();
			m_name = "ComponentBase";
		};

		virtual ~ComponentBase() = default;

		virtual const ComponentType GetType() const { return ComponentType::Undefined; }
		const std::string GetName() const { return m_name; }
		const uuids::uuid GetTypeID() const { return g_objectTypeUUID.at(GetType()).value(); }
		const uuids::uuid GetObjectID() const { return m_objectID; }
		void SetRenderID(uint32_t id) { m_renderID = id; }

		// todo. Objectに移動する
		//OBBData GetWorldOBBData()
		//{
		//	OBBData obbData{};

		//	glm::vec3 localExtent = (m_aabbMax - m_aabbMin) * 0.5f;		// ローカルAABBの半径
		//	glm::vec3 centerLocal = (m_aabbMax + m_aabbMin) * 0.5f;		// ローカルAABBの中心
		//	glm::vec3 scaleExtent = localExtent * m_transform.scale;	// スケール適用

		//	// 回転を適用
		//	glm::mat3 rotMat = glm::mat3_cast(glm::quat(glm::radians(m_transform.rot)));
		//	auto testX = rotMat * glm::vec3(1.0f, 0.0f, 0.0f);
		//	auto testY = rotMat * glm::vec3(0.0f, 1.0f, 0.0f);
		//	auto testZ = rotMat * glm::vec3(0.0f, 0.0f, 1.0f);
		//	obbData.axisX = glm::vec4(rotMat * glm::vec3(1.0f, 0.0f, 0.0f), scaleExtent.x);
		//	obbData.axisY = glm::vec4(rotMat * glm::vec3(0.0f, 1.0f, 0.0f), scaleExtent.y);
		//	obbData.axisZ = glm::vec4(rotMat * glm::vec3(0.0f, 0.0f, 1.0f), scaleExtent.z);

		//	// OBBの中心 (ワールド空間)
		//	obbData.center = glm::vec4(m_transform.pos + rotMat * (centerLocal * m_transform.scale), 0.0f);

		//	return obbData;
		//}

		// シリアルライズ
		virtual rapidjson::Value Serialize(rapidjson::Document::AllocatorType& allocator) const
		{
			rapidjson::Value json(rapidjson::Type::kObjectType);
			json.AddMember("id", rapidjson::Value(uuids::to_string(m_objectID).c_str(), allocator), allocator);
			json.AddMember("name", rapidjson::Value(m_name.c_str(), allocator), allocator);
			json.AddMember("typeID", rapidjson::Value(uuids::to_string(GetTypeID()).c_str(), allocator), allocator);

			return json;
		}

		// デシリアルライズ
		virtual void Deserialize(const rapidjson::Value& doc)
		{
			m_objectID = uuids::uuid::from_string(doc["id"].GetString()).value();
			m_name = doc["name"].GetString();
		}

		// IsCamera?
		bool IsCamera() const
		{
			return GetType() == ComponentType::FPSCamera;
		}

		virtual bool IsStaticMesh() const
		{
			return false;
		}

	protected:
		bool m_isReady;
		std::string m_name;
		uuids::uuid m_objectID;
		uint32_t m_renderID;
	};
};