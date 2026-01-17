// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"
#include "PropertyBase.h"
#include "PropertyEditorBase.h"

#include "FloatPropertyEditor.h"
#include "Vec3PropertyEditor.h"
#include "Vec4PropertyEditor.h"
#include "EnumPropertyEditor.h"

class PropertyEditorFactory
{
public:
	PropertyEditorFactory()
	{
		// float
		Register<float>([](void* o, const MyosotisFW::PropertyDesc& d, QWidget* p)
			{
				return std::make_unique<FloatPropertyEditor>(o, d, p);
			});
		// vec3
		Register<glm::vec3>([](void* o, const MyosotisFW::PropertyDesc& d, QWidget* p)
			{
				return std::make_unique<Vec3PropertyEditor>(o, d, p);
			});
		// vec4
		Register<glm::vec4>([](void* o, const MyosotisFW::PropertyDesc& d, QWidget* p)
			{
				return std::make_unique<Vec4PropertyEditor>(o, d, p);
			});
	}

	// エディタを生成するメイン関数
	std::unique_ptr<PropertyEditorBase> CreateEditor(void* obj, const MyosotisFW::PropertyDesc& desc, QWidget* parent)
	{
		// enum
		if (desc.enumItems != nullptr && desc.enumCount > 0)
		{
			return std::make_unique<EnumPropertyEditor>(obj, desc, parent);
		}

		auto it = m_creators.find(desc.type.id);
		if (it != m_creators.end()) {
			return it->second(obj, desc, parent);
		}
		return nullptr;
	}

private:
	template<typename T>
	void Register(std::function<std::unique_ptr<PropertyEditorBase>(void*, const MyosotisFW::PropertyDesc&, QWidget*)> func)
	{
		m_creators[MyosotisFW::PropertyType::g_propertyTypeMap.at(typeid(T)).id] = func;
	}

	std::unordered_map<uuids::uuid, std::function<std::unique_ptr<PropertyEditorBase>(void*, const MyosotisFW::PropertyDesc&, QWidget*)>> m_creators;
};