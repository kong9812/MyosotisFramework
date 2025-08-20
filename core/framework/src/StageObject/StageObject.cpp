// Copyright (c) 2025 kong9812
#pragma once
#include "StageObject.h"
#include "ComponentBase.h"
#include "ComponentType.h"
#include "ComponentFactory.h"
#include "FpsCamera.h"
#include "PrimitiveGeometry.h"
#include "CustomMesh.h"
#include "Skybox.h"
#include "InteriorObject.h"
#include "Camera.h"

namespace MyosotisFW
{
	using namespace System::Render;

	void StageObject::Update(const UpdateData& updateData, const Camera::CameraBase_ptr& mainCamera)
	{
		for (ComponentBase_ptr& component : m_components)
		{
			if (IsStaticMesh(component->GetType()))
			{
				StaticMesh_ptr staticMesh = Object_CastToStaticMesh(component);
				staticMesh->Update(updateData, mainCamera);
			}
			else if (component->GetType() == ComponentType::Skybox)
			{
				Skybox_ptr staticMesh = Object_CastToSkybox(component);
				staticMesh->Update(updateData, mainCamera);
			}
			else if (component->GetType() == ComponentType::InteriorObjectMesh)
			{
				InteriorObject_ptr staticMesh = Object_CastToInteriorObject(component);
				staticMesh->Update(updateData, mainCamera);
			}
		}

		for (StageObject_ptr& child : m_children)
		{
			child->Update(updateData, mainCamera);
		}
	}

	// カメラ持つ？
	const bool StageObject::HasCamera(bool findChildComponent) const
	{
		for (const auto& component : m_components)
		{
			if (component->HasCamera())
				return true;
		}
		if (findChildComponent)
		{
			for (const auto& child : m_children)
			{
				if (child->HasCamera())
					return true;
			}

		}
		return false;
	}

	ComponentBase_ptr StageObject::FindComponent(const ComponentType& type, const bool findChildComponent)
	{
		for (ComponentBase_ptr& component : m_components)
		{
			if (component->GetType() == type)
			{
				return component;
			}
		}
		if (findChildComponent)
		{
			for (StageObject_ptr& child : m_children)
			{
				ComponentBase_ptr foundComponent = child->FindComponent(type, findChildComponent);
				if (foundComponent)
					return foundComponent;
			}
		}
		return nullptr;
	}

	std::vector<ComponentBase_ptr> StageObject::FindAllComponents(const ComponentType& type, const bool findChildComponent)
	{
		std::vector<ComponentBase_ptr> foundComponents{};
		for (ComponentBase_ptr& component : m_components)
		{
			if (component->GetType() == type)
			{
				foundComponents.push_back(component);
			}
		}
		if (findChildComponent)
		{
			for (StageObject_ptr& child : m_children)
			{
				auto childComponents = child->FindAllComponents(type, findChildComponent);
				foundComponents.insert(foundComponents.end(), childComponents.begin(), childComponents.end());
			}
		}
		return foundComponents;
	}

	std::vector<ComponentBase_ptr> StageObject::GetAllComponents(bool findChildComponent)
	{
		std::vector<ComponentBase_ptr> allComponents = m_components;
		if (findChildComponent)
		{
			for (const StageObject_ptr& child : m_children)
			{
				auto childComponents = child->GetAllComponents(findChildComponent);
				allComponents.insert(allComponents.end(), childComponents.begin(), childComponents.end());
			}
		}
		return allComponents;
	}

	void StageObject::AddComponent(const ComponentBase_ptr& component)
	{
		m_components.push_back(component);
	}

	// シリアルライズ
	rapidjson::Value StageObject::Serialize(rapidjson::Document::AllocatorType& allocator) const
	{
		rapidjson::Value json(rapidjson::Type::kObjectType);
		json.AddMember("id", rapidjson::Value(uuids::to_string(m_objectID).c_str(), allocator), allocator);
		json.AddMember("name", rapidjson::Value(m_name.c_str(), allocator), allocator);

		SerializeVec3ToJson<glm::vec3>("pos", m_transform.pos, json, allocator);
		SerializeVec3ToJson<glm::vec3>("rot", m_transform.rot, json, allocator);
		SerializeVec3ToJson<glm::vec3>("scale", m_transform.scale, json, allocator);

		// もし子要素があれば
		rapidjson::Value childrenArray(rapidjson::Type::kArrayType);
		for (const auto& child : m_children)
		{
			childrenArray.PushBack(childrenArray.PushBack(child->Serialize(allocator), allocator), allocator);
		}
		json.AddMember("children", childrenArray, allocator);

		return json;
	}

	// デシリアルライズ
	StageObject* StageObject::Deserialize(const rapidjson::Value& doc)
	{
		m_objectID = uuids::uuid::from_string(doc["id"].GetString()).value();
		m_name = doc["name"].GetString();

		DeserializeVec3FromJson<glm::vec3>("pos", m_transform.pos, doc);
		DeserializeVec3FromJson<glm::vec3>("rot", m_transform.rot, doc);
		DeserializeVec3FromJson<glm::vec3>("scale", m_transform.scale, doc);

		// 子要素のデシリアル化
		if (doc.HasMember("children") && doc["children"].IsArray())
		{
			for (const auto& child : doc["children"].GetArray())
			{
				StageObject_ptr childObject = CreateStageObjectPointer();
				childObject->Deserialize(child);
				m_children.push_back(childObject);
			}
		}

		// コンポーネントのデシリアル化
		if (doc.HasMember("components") && doc["components"].IsArray())
		{
			for (const auto& comp : doc["components"].GetArray())
			{
				auto typeID = comp["typeID"].GetString();
				auto optType = uuids::uuid::from_string(typeID);
				ComponentType type = findComponentTypeFromTypeID(optType.value());
				ComponentBase_ptr component = System::ComponentFactory::CreateComponent(type);
				component->Deserialize(comp);
				m_components.push_back(component);
			}
		}
		return this;
	}
};