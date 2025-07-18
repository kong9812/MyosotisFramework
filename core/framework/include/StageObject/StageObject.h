// Copyright (c) 2025 kong9812
#pragma once
#include "ClassPointer.h"
#include "Structs.h"
#include "istduuid.h"
#include "iRapidJson.h"
#include "ComponentType.h"

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

	class StageObject;
	TYPEDEF_SHARED_PTR(StageObject);

	class StageObject
	{
	public:
		StageObject() :
			m_isReady(false),
			m_name("StageObject"),
			m_objectID(),
			m_renderID(0),
			m_transform({ glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f) }),
			m_children(),
			m_components()
		{
			m_objectID = hashMaker();
		}
		virtual ~StageObject() = default;

		const std::string GetName() const { return m_name; }
		const uuids::uuid GetObjectID() const { return m_objectID; }

		const glm::vec3 GetPos() const { return m_transform.pos; }
		const glm::vec3 GetRot() const { return m_transform.rot; }
		const glm::vec3 GetScale() const { return m_transform.scale; }
		const void SetPos(const glm::vec3& pos) { m_transform.pos = pos; }
		const void SetRot(const glm::vec3& rot) { m_transform.rot = rot; }
		const void SetScale(const glm::vec3& scale) { m_transform.scale = scale; }
		void SetRenderID(uint32_t id) { m_renderID = id; }

		void Update(const UpdateData& updateData, const RenderNS::Camera::CameraBase_ptr& mainCamera);
		const bool HasCamera(bool findChildComponent = false) const;

		ComponentBase_ptr FindComponent(const ComponentType& type, bool findChildComponent = false);
		std::vector<ComponentBase_ptr> FindAllComponents(const ComponentType& type, bool findChildComponent = false);
		std::vector< ComponentBase_ptr> GetAllComponents(bool findChildComponent = false);
		void AddComponent(const ComponentBase_ptr& component);

		// シリアルライズ
		virtual rapidjson::Value Serialize(rapidjson::Document::AllocatorType& allocator) const;
		// デシリアルライズ
		virtual StageObject* Deserialize(const rapidjson::Value& doc);

	protected:
		bool m_isReady;
		std::string m_name;
		uuids::uuid m_objectID;
		uint32_t m_renderID;

		Transform m_transform;
		std::vector<StageObject_ptr> m_children;
		std::vector<ComponentBase_ptr> m_components;
	};
};