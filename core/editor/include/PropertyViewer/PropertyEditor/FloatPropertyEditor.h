// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"
#include "PropertyEditorBase.h"

class FloatPropertyEditor : public PropertyEditorBase
{
public:
	FloatPropertyEditor(void* obj, const MyosotisFW::PropertyDesc& desc, QWidget* parent) :
		PropertyEditorBase(obj, desc, parent),
		m_doubleSpinBox(new QDoubleSpinBox(this))
	{
		QHBoxLayout* layout = new QHBoxLayout(this);
		layout->setContentsMargins(0, 0, 0, 0);

		m_doubleSpinBox->setRange(-999999.0, 999999.0);

		// UIが変更されたら Apply を呼ぶ
		connect(m_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
			[this](double val)
			{
				m_desc.apply(m_object, static_cast<float>(val), MyosotisFW::ChangeReason::UI_Preview);
			});

		layout->addWidget(m_doubleSpinBox);
		refreshUI();
	}

	void refreshUI() override
	{
		float val = std::get<float>(m_desc.get(m_object));
		m_doubleSpinBox->setValue(val);
	}

private:
	QDoubleSpinBox* m_doubleSpinBox;
};