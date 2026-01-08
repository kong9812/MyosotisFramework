// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"
#include "ComponentProperty.h"
#include "PropertyEditorBase.h"

#include "FloatPropertyEditor.h"

class PropertyEditorFactory
{
public:
	PropertyEditorFactory()
	{
		// float型のエディタを登録
		Register<float>([](void* o, const MyosotisFW::PropertyDesc& d, QWidget* p)
			{
				return std::make_unique<FloatPropertyEditor>(o, d, p);
			});
		// todo. 他の型を追加していくぞ！
	}

	// エディタを生成するメイン関数
	std::unique_ptr<PropertyEditorBase> CreateEditor(void* obj, const MyosotisFW::PropertyDesc& desc, QWidget* parent)
	{
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