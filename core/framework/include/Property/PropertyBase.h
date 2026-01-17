// Copyright (c) 2025 kong9812
#pragma once
#include <variant>
#include <string>
#include <typeindex>
#include <unordered_map>
#include "iglm.h"
#include "istduuid.h"

namespace MyosotisFW
{
	namespace PropertyType
	{
		struct PropertyTypeData
		{
			const char* name;
			uuids::uuid id;
		};

		static std::unordered_map<std::type_index, PropertyTypeData> g_propertyTypeMap =
		{
			// 文字列
			{ typeid(std::string), PropertyTypeData{"string", uuids::uuid::from_string("4186d7b6-3e2d-473a-aadf-ee356ace76b7").value()} },
			// 整数
			{ typeid(uint32_t), PropertyTypeData{"uint32_t", uuids::uuid::from_string("5c0a265b-dd66-40e5-987b-4334c63ea91c").value()} },
			{ typeid(int32_t), PropertyTypeData{"int32_t", uuids::uuid::from_string("f1f2420a-2418-453f-9d03-3514783bf9d9").value()} },
			{ typeid(uint64_t), PropertyTypeData{"uint64_t", uuids::uuid::from_string("db91a6d7-746b-4d69-8061-7aef76c9b008").value()} },
			{ typeid(int64_t), PropertyTypeData{"int64_t", uuids::uuid::from_string("4b39437c-3cd5-41be-8923-d44b6d14dcd6").value()} },
			// 小数
			{ typeid(float), PropertyTypeData{"float", uuids::uuid::from_string("61133c48-c857-4d2e-815e-a8c74df02473").value()} },
			{ typeid(double), PropertyTypeData{"double", uuids::uuid::from_string("cfbe0656-4833-4957-ae27-263bce01d39c").value()} },
			// ベクトル
			{ typeid(glm::vec2), PropertyTypeData{"vec2", uuids::uuid::from_string("63c759b0-42f1-4993-b9f7-b7148855afea").value()} },
			{ typeid(glm::vec3), PropertyTypeData{"vec3", uuids::uuid::from_string("827b890d-ec60-4edc-8fe4-75667cf50c2a").value()} },
			{ typeid(glm::vec4), PropertyTypeData{"vec4", uuids::uuid::from_string("fd418f41-f082-4f30-8199-4f565d3681c2").value()} },
			{ typeid(glm::ivec2), PropertyTypeData{"ivec2", uuids::uuid::from_string("8bb7b5a9-00e4-4a8a-871f-29a59fe536bd").value()} },
			{ typeid(glm::ivec3), PropertyTypeData{"ivec3", uuids::uuid::from_string("c8572c8a-86c3-4f04-89fc-310e897b3b1d").value()} },
			{ typeid(glm::ivec4), PropertyTypeData{"ivec4", uuids::uuid::from_string("01bf6008-681b-47da-8e6d-601d1eff7f55").value()} },
		};
	}

	using PropertyValue = std::variant<
		// 文字列
		std::string,
		// 整数
		uint32_t, int32_t, uint64_t, int64_t,
		// 小数
		float, double,
		// ベクトル型
		glm::vec2, glm::vec3, glm::vec4,
		glm::ivec2, glm::ivec3, glm::ivec4>;

	enum class PropertyFlags : uint32_t
	{
		None = 0,
		ReadOnly = 1 << 0,
	};

	enum class ChangeReason
	{
		UI_Preview,
		UI_Commit,
		UndoRedo,
		Deserialize,
		TrySet
	};

	using GetterFunction = PropertyValue(*)(const void* obj);
	using ApplyFunction = void(*)(void*, const PropertyValue& v, ChangeReason c);

	struct EnumItem
	{
		const char* name;
		int32_t value;	// todo.今後はcharとか他の型にも対応する必要がある
	};

	struct PropertyDesc
	{
		uuids::uuid id{};
		PropertyType::PropertyTypeData type{};
		PropertyFlags flags = PropertyFlags::None;
		const char* name = "";
		const char* category = "";

		GetterFunction get{};
		ApplyFunction apply{};

		// enum
		const EnumItem* enumItems = nullptr;
		size_t enumCount = 0;
	};

	struct PropertyTable
	{
		const PropertyTable* base = nullptr;
		const PropertyDesc* props = nullptr;
		size_t count = 0;

		template<class F>
		void ForEach(F&& f) const
		{
			if (base)
			{
				base->ForEach(f);
			}

			for (size_t i = 0; i < count; i++)
			{
				f(props[i]);
			}
		}

		const PropertyDesc* Find(uuids::uuid typeID) const
		{
			if (base)
			{
				if (const PropertyDesc* p = base->Find(typeID))
				{
					return p;
				}
			}
			for (size_t i = 0; i < count; i++)
			{
				if (props[i].type.id == typeID)
				{
					return &props[i];
				}
			}
			return nullptr;
		}
	};

	// 便利関数
	inline PropertyFlags operator|(PropertyFlags a, PropertyFlags b)
	{
		return static_cast<PropertyFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
	}
	inline bool HasPropertyFlag(PropertyFlags target, PropertyFlags find)
	{
		return (static_cast<uint32_t>(target) & static_cast<uint32_t>(find)) != 0;
	}

	template<class C, class T, T C::* Member>
	PropertyDesc MakeMemberProp(uuids::uuid id, const char* name, const char* category, PropertyFlags flags = PropertyFlags::None, ApplyFunction applyFunc = nullptr)
	{
		ApplyFunction apply = applyFunc ? applyFunc :
			[](void* obj, const PropertyValue& v, ChangeReason cr)
			{
				auto* c = static_cast<C*>(obj);
				c->*Member = std::get<T>(v);
			};

		return PropertyDesc{
			id,
			PropertyType::g_propertyTypeMap.at(typeid(T)),
			flags,
			name,
			category,

			[](const void* obj) -> PropertyValue
			{
				auto* c = static_cast<const C*>(obj);
				return c->*Member;
			},
			apply
		};
	}

	template<class T>
	PropertyDesc MakeProp(uuids::uuid id, const char* name, const char* category,
		PropertyFlags flags = PropertyFlags::None,
		GetterFunction getFunc = nullptr,
		ApplyFunction applyFunc = nullptr)
	{
		return PropertyDesc{
			id,
			PropertyType::g_propertyTypeMap.at(typeid(T)),
			flags,
			name,
			category,

			getFunc,
			applyFunc
		};
	}

	template<class C, class T, T C::* Member>
	PropertyDesc MakeEnumProp(uuids::uuid id, const char* name, const char* category,
		const EnumItem* items, size_t itemCount,
		PropertyFlags flags = PropertyFlags::None, ApplyFunction applyFunc = nullptr)
	{
		ApplyFunction apply = applyFunc ? applyFunc :
			[](void* obj, const PropertyValue& v, ChangeReason cr)
			{
				auto* c = static_cast<C*>(obj);
				c->*Member = static_cast<T>(std::get<int32_t>(v));	// 仮: int32_t 固定
			};

		PropertyDesc desc = PropertyDesc{
			id,
			PropertyType::g_propertyTypeMap.at(typeid(int32_t)),	// 仮: int32_t 固定
			flags,
			name,
			category,

			[](const void* obj) -> PropertyValue
			{
				auto* c = static_cast<const C*>(obj);
				return static_cast<int32_t>(c->*Member);		// 仮: int32_t 固定
			},
			apply
		};

		desc.enumItems = items;
		desc.enumCount = itemCount;
		return desc;
	}
};