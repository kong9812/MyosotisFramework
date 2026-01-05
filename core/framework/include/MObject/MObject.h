// Copyright (c) 2025 kong9812
#pragma once
#include <vector>
#include "ClassPointer.h"
#include "ObjectInfo.h"
#include "UpdateData.h"
#include "VBDispatchInfo.h"
#include "Transform.h"
#include "istduuid.h"
#include "iRapidJson.h"
#include "ComponentType.h"
#include "TLASInstance.h"

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
	using MObjectList = std::vector<MObject_ptr>;
	using MObjectListPtr = std::shared_ptr<MObjectList>;

	class MObject : public std::enable_shared_from_this<MObject>
	{
	public:
		MObject() :
			m_isReady(false),
			m_name("MObject"),
			m_uuid(),
			m_renderID(0),
			m_children(),
			m_components(),
			m_dirty(true)
		{
			m_uuid = uuids::hashMaker();
		}
		virtual ~MObject() = default;

		const std::string GetName() const { return m_name; }
		void SetName(std::string name) { m_name = name; }

		const uuids::uuid GetUUID() const { return m_uuid; }
		const uint32_t GetObjectID() const { return m_objectInfo->objectID; }

		MObject_ptr GetParent() const { return m_parent; }
		std::vector<MObject_ptr> GetChildren() const { return m_children; }

		void AddChild(MObject_ptr child);
		void RemoveChild(MObject_ptr child);
		void SetParent(MObject_ptr parent);

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
		void SetTLASInstance(TLASInstance_ptr tlasInstance) { m_tlasInstance = tlasInstance; }
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
		uuids::uuid m_uuid;
		uint32_t m_renderID;

		bool m_dirty;
		std::function<void(void)> m_meshChangedCallback;
		std::function<void(void)> m_transformChangedCallback;

		ObjectInfo_ptr m_objectInfo;
		TLASInstance_ptr m_tlasInstance;

		MObject_ptr m_parent;
		std::vector<MObject_ptr> m_children;
		std::vector<ComponentBase_ptr> m_components;
	};
};