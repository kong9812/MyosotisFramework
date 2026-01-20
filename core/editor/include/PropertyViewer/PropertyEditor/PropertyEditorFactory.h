// Copyright (c) 2025 kong9812
#pragma once
#include <functional>

#include "iqt.h"
#include "PropertyBase.h"
#include "PropertyEditorBase.h"

#include "StringPropertyEditor.h"
#include "FilePathPropertyEditor.h"
#include "FloatPropertyEditor.h"
#include "Vec2PropertyEditor.h"
#include "Vec3PropertyEditor.h"
#include "Vec4PropertyEditor.h"
#include "EnumPropertyEditor.h"

class PropertyEditorFactory
{
public:
	using PropertyChangedCallback = std::function<void(void*, const MyosotisFW::PropertyDesc&, MyosotisFW::PropertyDesc::ChangeReason)>;

	PropertyEditorFactory()
	{
		// string
		Register<std::string>([](void* o, const MyosotisFW::PropertyDesc& d, QWidget* p)
			{
				return std::make_unique<StringPropertyEditor>(o, d, p);
			});
		// FilePath
		Register<MyosotisFW::FilePath>([](void* o, const MyosotisFW::PropertyDesc& d, QWidget* p)
			{
				QString base = d.filePathItem ? d.filePathItem->basePass : "";
				QString filter = d.filePathItem ? d.filePathItem->filter : "";
				QString def = d.filePathItem ? d.filePathItem->defaultDir : "";
				return std::make_unique<FilePathPropertyEditor>(o, d, p, base, filter, def);
			});
		// float
		Register<float>([](void* o, const MyosotisFW::PropertyDesc& d, QWidget* p)
			{
				return std::make_unique<FloatPropertyEditor>(o, d, p);
			});
		// vec2
		Register<glm::vec2>([](void* o, const MyosotisFW::PropertyDesc& d, QWidget* p)
			{
				return std::make_unique<Vec2PropertyEditor>(o, d, p);
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
	std::unique_ptr<PropertyEditorBase> CreateEditor(void* obj,
		const MyosotisFW::PropertyDesc& desc,
		QWidget* parent,
		PropertyChangedCallback callback = nullptr)
	{
		std::unique_ptr<PropertyEditorBase> editor = nullptr;

		// enum
		if (desc.enumItems != nullptr && desc.enumCount > 0)
		{
			editor = std::make_unique<EnumPropertyEditor>(obj, desc, parent);
		}
		else
		{
			auto it = m_creators.find(desc.type.id);
			if (it != m_creators.end()) {
				editor = it->second(obj, desc, parent);
			}
		}

		// Callback登録
		if (editor && callback)
		{
			// PropertyEditorBase::valueChanged シグナルを外部のコールバックに接続
			QObject::connect(editor.get(), &PropertyEditorBase::valueChanged, callback);
		}

		return editor;
	}

private:
	template<typename T>
	void Register(std::function<std::unique_ptr<PropertyEditorBase>(void*, const MyosotisFW::PropertyDesc&, QWidget*)> func)
	{
		m_creators[MyosotisFW::PropertyType::g_propertyTypeMap.at(typeid(T)).id] = func;
	}

	std::unordered_map<uuids::uuid, std::function<std::unique_ptr<PropertyEditorBase>(void*, const MyosotisFW::PropertyDesc&, QWidget*)>> m_creators;
};