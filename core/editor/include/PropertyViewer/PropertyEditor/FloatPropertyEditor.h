// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"
#include "PropertyEditorBase.h"
#include "StrictWheelDoubleSpinBox.h"

class FloatPropertyEditor : public PropertyEditorBase
{
public:
	FloatPropertyEditor(void* obj, const MyosotisFW::PropertyDesc& desc, QWidget* parent) :
		PropertyEditorBase(obj, desc, parent),
		m_doubleSpinBox(new StrictWheelDoubleSpinBox(this))
	{
		QHBoxLayout* layout = new QHBoxLayout(this);
		layout->setContentsMargins(0, 0, 0, 0);

		m_doubleSpinBox->setRange(-999999.0, 999999.0);

		// UIが変更されたら Apply を呼ぶ
		connect(m_doubleSpinBox, QOverload<double>::of(&StrictWheelDoubleSpinBox::valueChanged),
			[this](double val)
			{
				if (m_desc.apply)
				{
					m_desc.apply(m_object, static_cast<float>(val), MyosotisFW::PropertyDesc::ChangeReason::UI_Preview);
				}
			});

		layout->addWidget(m_doubleSpinBox);
		refreshUI();
	}

	void refreshUI() override
	{
		if (!m_desc.get) return;

		float val = std::get<float>(m_desc.get(m_object));

		{// UI更新時に valueChanged シグナルが発生して apply が再送されないようにブロックする
			m_doubleSpinBox->blockSignals(true);
			m_doubleSpinBox->setValue(static_cast<double>(val));
			m_doubleSpinBox->blockSignals(false);
		}
	}

private:
	StrictWheelDoubleSpinBox* m_doubleSpinBox;
};