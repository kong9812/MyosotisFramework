// Copyright (c) 2025 kong9812
#pragma once
#include "ClassPointer.h"
#include "Structs.h"
#include "istduuid.h"
#include "iRapidJson.h"
#include "ComponentBase.h"
#include "Camera.h"

namespace MyosotisFW
{
	namespace System::Render {}
	namespace RenderNS = System::Render;

	class StageObject;
	TYPEDEF_SHARED_PTR(StageObject)

		class StageObject
	{
	public:
		StageObject() :
			m_transfrom({ glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f) }),
			m_objectID(),
			m_isReady(false),
			m_name("StageObject")
		{
			m_objectID = hashMaker();
		}
		virtual ~StageObject() = default;

		const std::string GetName() const { return m_name; }
		const uuids::uuid GetObjectID() const { return m_objectID; }

		const glm::vec3 GetPos() const { return m_transfrom.pos; }
		const glm::vec3 GetRot() const { return m_transfrom.rot; }
		const glm::vec3 GetScale() const { return m_transfrom.scale; }
		const void SetPos(const glm::vec3& pos) { m_transfrom.pos = pos; }
		const void SetRot(const glm::vec3& rot) { m_transfrom.rot = rot; }
		const void SetScale(const glm::vec3& scale) { m_transfrom.scale = scale; }
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

		Transform m_transfrom;
		std::vector<StageObject_ptr> m_children;
		std::vector<ComponentBase_ptr> m_components;
	};
};