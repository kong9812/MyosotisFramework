// Copyright (c) 2025 kong9812
#pragma once
#include "ClassPointer.h"
#include "ComponentType.h"
#include "Transform.h"
#include "istduuid.h"
#include "iRapidJson.h"
#include "ComponentFactory.h"
#include "TLASInstance.h"
#include "PropertyBase.h"

namespace MyosotisFW
{
	class ComponentBase;
	TYPEDEF_SHARED_PTR_ARGS(ComponentBase);
	using ComponentBaseHandle = std::weak_ptr<ComponentBase>;				// 外部参照用
	using ComponentBaseList = std::vector<ComponentBase_ptr>;
	using ComponentBaseHandleList = std::vector<ComponentBaseHandle>;		// 外部参照用

	class ComponentBase
	{
	public:
		ComponentBase(const uint32_t objectID) :
			m_objectID(objectID),
			m_isReady(false),
			m_name()
		{
			m_name = "ComponentBase";
		};

		virtual ~ComponentBase() = default;

		virtual const ComponentType GetType() const { return ComponentType::Undefined; }
		const std::string GetName() const { return m_name; }
		const uuids::uuid GetTypeID() const { return g_objectTypeUUID.at(GetType()).value(); }
		const uint32_t GetHashObjectID() const { return m_objectID; }
		void SetTLASInstance(TLASInstance_ptr tlasInstance) { m_tlasInstance = tlasInstance; }

		// シリアルライズ
		virtual rapidjson::Value Serialize(rapidjson::Document::AllocatorType& allocator) const
		{
			rapidjson::Value json(rapidjson::Type::kObjectType);
			json.AddMember("id", rapidjson::Value(m_objectID), allocator);
			json.AddMember("name", rapidjson::Value(m_name.c_str(), allocator), allocator);
			json.AddMember("typeID", rapidjson::Value(uuids::to_string(GetTypeID()).c_str(), allocator), allocator);

			return json;
		}

		// デシリアルライズ
		virtual void Deserialize(const rapidjson::Value& doc)
		{
			m_objectID = doc["id"].GetUint();
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
		uint32_t m_objectID;

		TLASInstance_ptr m_tlasInstance;

	public:
		// ComponentProperty
		static const PropertyTable& StaticPropertyTable()
		{
			static const PropertyTable table{ nullptr, nullptr, 0 };
			return table;
		}
		virtual const PropertyTable& GetPropertyTable() const
		{
			return StaticPropertyTable();
		}

		bool TryGet(uuids::uuid id, PropertyDesc::PropertyValue& v) const
		{
			if (const PropertyDesc* p = GetPropertyTable().Find(id))
			{
				v = p->get(this);
				return true;
			}
			return false;
		}

		bool TrySet(uuids::uuid id, PropertyDesc::PropertyValue& v)
		{
			if (const PropertyDesc* p = GetPropertyTable().Find(id))
			{
				if (HasPropertyFlag(p->flags, PropertyDesc::PropertyFlags::ReadOnly)) return false;
				p->apply(this, v, PropertyDesc::ChangeReason::TrySet);
				OnPropertyChanged(id);
				return true;
			}
			return false;
		}
	protected:
		virtual void OnPropertyChanged(uuids::uuid propertyID) {}

	};
};