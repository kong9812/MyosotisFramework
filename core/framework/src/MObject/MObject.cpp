// Copyright (c) 2025 kong9812
#pragma once
#include "MObject.h"
#include "ComponentBase.h"
#include "ComponentType.h"
#include "ComponentFactory.h"
#include "FpsCamera.h"
#include "PrimitiveGeometry.h"
#include "CustomMesh.h"
#include "Camera.h"

namespace MyosotisFW
{
	using namespace System::Render;

	const uint32_t MObject::GetMeshCount() const
	{
		auto it = std::find_if(m_components.begin(), m_components.end(),
			[&](const ComponentBase_ptr& existingComponent)
			{
				return existingComponent->IsStaticMesh();
			});
		if (it != m_components.end())
		{
			StaticMesh_ptr staticMesh = Object_CastToStaticMesh(*it);
			return staticMesh->GetMeshCount();
		}
		return 0;
	}

	const std::vector<uint32_t> MObject::GetMeshID() const
	{
		auto it = std::find_if(m_components.begin(), m_components.end(),
			[&](const ComponentBase_ptr& existingComponent)
			{
				return existingComponent->IsStaticMesh();
			});
		if (it != m_components.end())
		{
			StaticMesh_ptr staticMesh = Object_CastToStaticMesh(*it);
			return staticMesh->GetMeshID();
		}
		return {};
	}

	const std::vector<VBDispatchInfo> MObject::GetVBDispatchInfo() const
	{
		auto it = std::find_if(m_components.begin(), m_components.end(),
			[&](const ComponentBase_ptr& existingComponent)
			{
				return existingComponent->IsStaticMesh();
			});
		if (it != m_components.end())
		{
			StaticMesh_ptr staticMesh = Object_CastToStaticMesh(*it);
			return staticMesh->GetVBDispatchInfo();
		}

		return {};
	}

	bool MObject::Update(const UpdateData& updateData, const Camera::CameraBase_ptr& mainCamera)
	{
		for (ComponentBase_ptr& component : m_components)
		{
			if (component->IsStaticMesh())
			{
				StaticMesh_ptr staticMesh = Object_CastToStaticMesh(component);
				staticMesh->Update(updateData, mainCamera);
			}
		}

		for (MObject_ptr& child : m_children)
		{
			child->Update(updateData, mainCamera);
		}

		if (m_dirty)
		{
			m_objectInfo->model = glm::translate(glm::mat4(1.0f), glm::vec3(m_objectInfo->transform.pos));
			m_objectInfo->model = glm::rotate(m_objectInfo->model, glm::radians(m_objectInfo->transform.rot.x), glm::vec3(1.0f, 0.0f, 0.0f));
			m_objectInfo->model = glm::rotate(m_objectInfo->model, glm::radians(m_objectInfo->transform.rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
			m_objectInfo->model = glm::rotate(m_objectInfo->model, glm::radians(m_objectInfo->transform.rot.z), glm::vec3(0.0f, 0.0f, 1.0f));
			m_objectInfo->model = glm::scale(m_objectInfo->model, glm::vec3(m_objectInfo->transform.scale));
			m_tlasInstanceInfo->model = m_objectInfo->model;
			m_tlasInstanceInfo->meshID = GetMeshID();
			m_dirty = false;
			return true;
		}
		return false;
	}

	// カメラ持つ？
	const bool MObject::IsCamera(bool findChildComponent) const
	{
		for (const auto& component : m_components)
		{
			if (component->IsCamera())
				return true;
		}
		if (findChildComponent)
		{
			for (const auto& child : m_children)
			{
				if (child->IsCamera())
					return true;
			}

		}
		return false;
	}

	ComponentBase_ptr MObject::FindComponent(const ComponentType& type, const bool findChildComponent)
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
			for (MObject_ptr& child : m_children)
			{
				ComponentBase_ptr foundComponent = child->FindComponent(type, findChildComponent);
				if (foundComponent)
					return foundComponent;
			}
		}
		return nullptr;
	}

	std::vector<ComponentBase_ptr> MObject::FindAllComponents(const ComponentType& type, const bool findChildComponent)
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
			for (MObject_ptr& child : m_children)
			{
				auto childComponents = child->FindAllComponents(type, findChildComponent);
				foundComponents.insert(foundComponents.end(), childComponents.begin(), childComponents.end());
			}
		}
		return foundComponents;
	}

	std::vector<ComponentBase_ptr> MObject::GetAllComponents(bool findChildComponent)
	{
		std::vector<ComponentBase_ptr> allComponents = m_components;
		if (findChildComponent)
		{
			for (const MObject_ptr& child : m_children)
			{
				auto childComponents = child->GetAllComponents(findChildComponent);
				allComponents.insert(allComponents.end(), childComponents.begin(), childComponents.end());
			}
		}
		return allComponents;
	}

	void MObject::AddComponent(const ComponentBase_ptr& component)
	{
		// 複数StaticMeshを許可しない
		auto it = std::find_if(m_components.begin(), m_components.end(),
			[&](const ComponentBase_ptr& existingComponent)
			{
				return existingComponent->IsStaticMesh();
			});
		ASSERT(it == m_components.end(), "ERROR!! Unable to add more than one static mesh component to an object.");

		component->SetTLASInstanceInfo(m_tlasInstanceInfo);
		m_components.push_back(component);
	}

	// シリアルライズ
	rapidjson::Value MObject::Serialize(rapidjson::Document::AllocatorType& allocator) const
	{
		rapidjson::Value json(rapidjson::Type::kObjectType);
		json.AddMember("id", rapidjson::Value(uuids::to_string(m_objectID).c_str(), allocator), allocator);
		json.AddMember("name", rapidjson::Value(m_name.c_str(), allocator), allocator);

		SerializeVec3ToJson<glm::vec4>("pos", m_objectInfo->transform.pos, json, allocator);
		SerializeVec3ToJson<glm::vec4>("rot", m_objectInfo->transform.rot, json, allocator);
		SerializeVec3ToJson<glm::vec4>("scale", m_objectInfo->transform.scale, json, allocator);

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
	MObject* MObject::Deserialize(const rapidjson::Value& doc)
	{
		m_objectID = uuids::uuid::from_string(doc["id"].GetString()).value();
		m_name = doc["name"].GetString();

		DeserializeVec3FromJson<glm::vec4>("pos", m_objectInfo->transform.pos, doc);
		DeserializeVec3FromJson<glm::vec4>("rot", m_objectInfo->transform.rot, doc);
		DeserializeVec3FromJson<glm::vec4>("scale", m_objectInfo->transform.scale, doc);

		// 子要素のデシリアル化
		if (doc.HasMember("children") && doc["children"].IsArray())
		{
			for (const auto& child : doc["children"].GetArray())
			{
				MObject_ptr childObject = CreateMObjectPointer();
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
				ComponentBase_ptr component = System::ComponentFactory::CreateComponent(m_objectInfo->objectID, type, m_meshChangedCallback);
				component->Deserialize(comp);
				m_components.push_back(component);
			}
		}
		return this;
	}
};