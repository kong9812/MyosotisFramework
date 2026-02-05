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

	MObject::MObject() :
		m_isReady(false),
		m_name("MObject"),
		m_uuid(uuids::hashMaker()),
		m_children(),
		m_components(),
		m_staticMeshHandles(),
		m_dirty(true) {
	}

	void MObject::AddChild(MObject_ptr child)
	{
		if (!child || child.get() == this) return;

		// すでに別の親がいる場合は、古い親から切り離す
		if (MObject_ptr oldParent = child->GetParent())
		{
			oldParent->RemoveChild(child);
		}

		// 自分の子リストに追加
		m_children.push_back(child);

		// 子に対して自分を親としてセット
		// enable_shared_from_thisで自分のSharedPtr発行
		child->SetParent(shared_from_this());
	}

	void MObject::RemoveChild(MObject_ptr child)
	{
		if (!child) return;

		// リストから削除
		auto it = std::remove(m_children.begin(), m_children.end(), child);
		if (it != m_children.end())
		{
			m_children.erase(it, m_children.end());
			// 絶縁
			child->SetParent(nullptr);
		}
	}

	void MObject::SetParent(MObject_ptr parent)
	{
		m_parent = parent;
	}

	const uint32_t MObject::GetMeshCount() const
	{
		auto it = std::find_if(m_components.raw.begin(), m_components.raw.end(),
			[&](const ComponentBase_ptr& existingComponent)
			{
				return existingComponent->IsStaticMesh();
			});
		if (it != m_components.raw.end())
		{
			StaticMesh_ptr staticMesh = Object_CastToStaticMesh(*it);
			return staticMesh->GetMeshCount();
		}
		return 0;
	}

	const std::vector<uint32_t> MObject::GetMeshID() const
	{
		auto it = std::find_if(m_components.raw.begin(), m_components.raw.end(),
			[&](const ComponentBase_ptr& existingComponent)
			{
				return existingComponent->IsStaticMesh();
			});
		if (it != m_components.raw.end())
		{
			StaticMesh_ptr staticMesh = Object_CastToStaticMesh(*it);
			return staticMesh->GetMeshID();
		}
		return {};
	}

	const std::vector<VBDispatchInfo> MObject::GetVBDispatchInfo() const
	{
		auto it = std::find_if(m_components.raw.begin(), m_components.raw.end(),
			[&](const ComponentBase_ptr& existingComponent)
			{
				return existingComponent->IsStaticMesh();
			});
		if (it != m_components.raw.end())
		{
			StaticMesh_ptr staticMesh = Object_CastToStaticMesh(*it);
			return staticMesh->GetVBDispatchInfo();
		}

		return {};
	}

	bool MObject::Update(const UpdateData& updateData, const Camera::CameraBase_ptr& mainCamera)
	{
		for (ComponentBase_ptr& component : m_components.raw)
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
			m_tlasInstance->model = m_objectInfo->model;
			m_tlasInstance->meshID = GetMeshID();
			m_dirty = false;
			return true;
		}
		return false;
	}

	// カメラ持つ？
	const bool MObject::IsCamera(bool findChildComponent) const
	{
		for (const ComponentBase_ptr& component : m_components.raw)
		{
			if (component->IsCamera())
				return true;
		}
		if (findChildComponent)
		{
			for (const MObject_ptr& child : m_children)
			{
				if (child->IsCamera())
					return true;
			}

		}
		return false;
	}

	ComponentBaseHandle* MObject::FindComponent(const ComponentType& type, const bool findChildComponent)
	{
		for (ComponentBaseHandle& handle : m_components.handles)
		{
			if (handle.lock()->GetType() == type)
			{
				return &handle;
			}
		}
		if (findChildComponent)
		{
			for (MObject_ptr& child : m_children)
			{
				ComponentBaseHandle* foundHandle = child->FindComponent(type, findChildComponent);
				if (foundHandle)
					return foundHandle;
			}
		}
		return nullptr;
	}

	ComponentBaseHandleList MObject::FindAllComponents(const ComponentType& type, const bool findChildComponent)
	{
		ComponentBaseHandleList foundHandle{};
		for (ComponentBaseHandle& handle : m_components.handles)
		{
			if (handle.lock()->GetType() == type)
			{
				foundHandle.push_back(handle);
			}
		}
		if (findChildComponent)
		{
			for (MObject_ptr& child : m_children)
			{
				ComponentBaseHandleList childComponents = child->FindAllComponents(type, findChildComponent);
				foundHandle.insert(foundHandle.end(), childComponents.begin(), childComponents.end());
			}
		}
		return foundHandle;
	}

	ComponentBaseHandleList MObject::GetAllComponents(bool findChildComponent)
	{
		ComponentBaseHandleList allHandles = m_components.handles;
		if (findChildComponent)
		{
			for (const MObject_ptr& child : m_children)
			{
				ComponentBaseHandleList childHandles = child->GetAllComponents(findChildComponent);
				allHandles.insert(allHandles.end(), childHandles.begin(), childHandles.end());
			}
		}
		return allHandles;
	}

	void MObject::AddComponent(const ComponentBase_ptr& component)
	{
		// 複数StaticMeshを許可しない
		auto it = std::find_if(m_components.raw.begin(), m_components.raw.end(),
			[&](const ComponentBase_ptr& existingComponent)
			{
				return existingComponent->IsStaticMesh();
			});
		ASSERT(it == m_components.raw.end(), "ERROR!! Unable to add more than one static mesh component to an object.");

		component->SetTLASInstance(m_tlasInstance);
		m_components.push_back(component);
	}

	// シリアルライズ
	rapidjson::Value MObject::Serialize(rapidjson::Document::AllocatorType& allocator) const
	{
		rapidjson::Value json(rapidjson::Type::kObjectType);
		json.AddMember("id", rapidjson::Value(uuids::to_string(m_uuid).c_str(), allocator), allocator);
		json.AddMember("name", rapidjson::Value(m_name.c_str(), allocator), allocator);

		SerializeVec3ToJson<glm::vec4>("pos", m_objectInfo->transform.pos, json, allocator);
		SerializeVec3ToJson<glm::vec4>("rot", m_objectInfo->transform.rot, json, allocator);
		SerializeVec3ToJson<glm::vec4>("scale", m_objectInfo->transform.scale, json, allocator);

		// コンポーネント
		rapidjson::Value componentArray(rapidjson::Type::kArrayType);
		for (const ComponentBase_ptr& component : m_components.raw)
		{
			componentArray.PushBack(component->Serialize(allocator), allocator);
		}
		json.AddMember("component", componentArray, allocator);

		// もし子要素があれば
		rapidjson::Value childrenArray(rapidjson::Type::kArrayType);
		for (const MObject_ptr& child : m_children)
		{
			childrenArray.PushBack(child->Serialize(allocator), allocator);
		}
		json.AddMember("children", childrenArray, allocator);

		return json;
	}

	// デシリアルライズ
	MObject* MObject::Deserialize(const rapidjson::Value& doc)
	{
		m_uuid = uuids::uuid::from_string(doc["id"].GetString()).value();
		m_name = doc["name"].GetString();

		DeserializeVec3FromJson<glm::vec4>("pos", m_objectInfo->transform.pos, doc);
		DeserializeVec3FromJson<glm::vec4>("rot", m_objectInfo->transform.rot, doc);
		DeserializeVec3FromJson<glm::vec4>("scale", m_objectInfo->transform.scale, doc);

		// コンポーネントのデシリアル化
		if (doc.HasMember("components") && doc["components"].IsArray())
		{
			for (const rapidjson::Value& comp : doc["components"].GetArray())
			{
				const char* typeID = comp["typeID"].GetString();
				std::optional<uuids::uuid> optType = uuids::uuid::from_string(typeID);
				ComponentType type = findComponentTypeFromTypeID(optType.value());
				ComponentBase_ptr component = System::ComponentFactory::CreateComponent(m_objectInfo->objectID, type, m_meshChangedCallback);
				component->Deserialize(comp);
				m_components.push_back(component);
			}
		}

		// 子要素のデシリアル化
		if (doc.HasMember("children") && doc["children"].IsArray())
		{
			for (const rapidjson::Value& child : doc["children"].GetArray())
			{
				MObject_ptr childObject = CreateMObjectPointer();
				childObject->Deserialize(child);
				m_children.push_back(childObject);
			}
		}
		return this;
	}
};