// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"
#include "PropertyBase.h"

class PropertyEditorBase : public QWidget
{
	Q_OBJECT
public:
	PropertyEditorBase(void* obj, const MyosotisFW::PropertyDesc& desc, QWidget* parent)
		: QWidget(parent), m_object(obj), m_desc(desc) {
	}
	virtual ~PropertyEditorBase() = default;

	// UIの値を現在のオブジェクトの状態に更新
	virtual void refreshUI() = 0;

signals:
	void valueChanged(void* obj, const MyosotisFW::PropertyDesc& desc, MyosotisFW::PropertyDesc::ChangeReason reason);

protected:
	void* m_object;
	MyosotisFW::PropertyDesc m_desc;
};