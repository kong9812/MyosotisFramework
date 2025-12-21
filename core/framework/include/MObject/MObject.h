// Copyright (c) 2025 kong9812
#pragma once
#include "ClassPointer.h"
#include "Structs.h"
#include "ObjectInfo.h"
#include "VBDispatchInfo.h"
#include "Transform.h"
#include "istduuid.h"
#include "iRapidJson.h"
#include "ComponentType.h"
#include "TLASInstanceInfo.h"

namespace MyosotisFW
{
	namespace System::Render {}
	namespace RenderNS = System::Render;

	// 前方宣言
	class ComponentBase;
	TYPEDEF_SHARED_PTR_FWD(ComponentBase);
	namespace System::Render::Camera
	{
		class CameraBase;
		TYPEDEF_SHARED_PTR_FWD(CameraBase);
	}

	class MObject;
	TYPEDEF_SHARED_PTR(MObject);

	class MObject
	{
	public:
		MObject() :
			m_isReady(false),
			m_name("MObject"),
			m_objectID(),
			m_renderID(0),
			m_children(),
			m_components(),
			m_dirty(true)
		{
			m_objectID = hashMaker();
		}
		virtual ~MObject() = default;

		const std::string GetName() const { return m_name; }

		const uuids::uuid GetHashObjectID() const { return m_objectID; }
		const uint32_t GetObjectID() const { return m_objectInfo->objectID; }

		const glm::vec4 GetPos() const { return m_objectInfo->transform.pos; }
		const glm::vec4 GetRot() const { return m_objectInfo->transform.rot; }
		const glm::vec4 GetScale() const { return m_objectInfo->transform.scale; }
		const void SetPos(const glm::vec3& pos) { m_objectInfo->transform.pos = glm::vec4(pos, 0.0f); m_transformChangedCallback(); m_dirty = true; }
		const void SetRot(const glm::vec3& rot) { m_objectInfo->transform.rot = glm::vec4(rot, 0.0f); m_transformChangedCallback(); m_dirty = true; }
		const void SetScale(const glm::vec3& scale) { m_objectInfo->transform.scale = glm::vec4(scale, 0.0f); m_transformChangedCallback(); m_dirty = true; }
		void SetRenderID(uint32_t id) { m_renderID = id; }
		void SetObjectInfo(ObjectInfo_ptr objectInfo) { m_objectInfo = objectInfo; }
		void SetMeshChangedCallback(const std::function<void(void)>& callback) { m_meshChangedCallback = callback; }
		void SetTransformChangedCallback(const std::function<void(void)>& callback) { m_transformChangedCallback = callback; }
		std::function<void(void)> GetMeshChangedCallback() const { return m_meshChangedCallback; }
		void SetTLASInstanceInfo(TLASInstanceInfo& tlasInstanceInfo) { m_tlasInstanceInfo = &tlasInstanceInfo; }
		std::function<void(void)> GetTransformChangedCallback() const { return m_transformChangedCallback; }

		const uint32_t GetMeshCount() const;
		const std::vector<uint32_t> GetMeshID()const;

		const ObjectInfo& GetObjectInfo() const { return *m_objectInfo; }
		const std::vector<VBDispatchInfo> GetVBDispatchInfo() const;

		bool Update(const UpdateData& updateData, const RenderNS::Camera::CameraBase_ptr& mainCamera);
		const bool IsCamera(bool findChildComponent = false) const;

		ComponentBase_ptr FindComponent(const ComponentType& type, const bool findChildComponent = false);
		std::vector<ComponentBase_ptr> FindAllComponents(const ComponentType& type, const bool findChildComponent = false);
		std::vector<ComponentBase_ptr> GetAllComponents(bool findChildComponent = false);
		void AddComponent(const ComponentBase_ptr& component);

		// シリアルライズ
		virtual rapidjson::Value Serialize(rapidjson::Document::AllocatorType& allocator) const;
		// デシリアルライズ
		virtual MObject* Deserialize(const rapidjson::Value& doc);

	protected:
		bool m_isReady;
		std::string m_name;
		uuids::uuid m_objectID;
		uint32_t m_renderID;

		bool m_dirty;
		std::function<void(void)> m_meshChangedCallback;
		std::function<void(void)> m_transformChangedCallback;

		ObjectInfo_ptr m_objectInfo;
		std::vector<MObject_ptr> m_children;
		std::vector<ComponentBase_ptr> m_components;

		TLASInstanceInfo* m_tlasInstanceInfo;
	};
};