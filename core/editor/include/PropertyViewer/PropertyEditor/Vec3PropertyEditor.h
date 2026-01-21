// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"
#include "PropertyEditorBase.h"
#include "StrictWheelDoubleSpinBox.h"

class Vec3PropertyEditor : public PropertyEditorBase
{
public:
	Vec3PropertyEditor(void* obj, const MyosotisFW::PropertyDesc& desc, QWidget* parent) :
		PropertyEditorBase(obj, desc, parent),
		m_doubleSpinBoxX(new StrictWheelDoubleSpinBox(this)),
		m_doubleSpinBoxY(new StrictWheelDoubleSpinBox(this)),
		m_doubleSpinBoxZ(new StrictWheelDoubleSpinBox(this))
	{
		QHBoxLayout* layout = new QHBoxLayout(this);
		layout->setContentsMargins(0, 0, 0, 0);

		// 読み取り専用の対応
		if (MyosotisFW::HasPropertyFlag(desc.flags, MyosotisFW::PropertyDesc::PropertyFlags::ReadOnly))
		{
			m_doubleSpinBoxX->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
			m_doubleSpinBoxX->setReadOnly(true);
			m_doubleSpinBoxX->setStyleSheet("QDoubleSpinBox { background: transparent; border: none; padding: 2px; }");

			m_doubleSpinBoxY->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
			m_doubleSpinBoxY->setReadOnly(true);
			m_doubleSpinBoxY->setStyleSheet("QDoubleSpinBox { background: transparent; border: none; padding: 2px; }");

			m_doubleSpinBoxZ->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
			m_doubleSpinBoxZ->setReadOnly(true);
			m_doubleSpinBoxZ->setStyleSheet("QDoubleSpinBox { background: transparent; border: none; padding: 2px; }");
		}

		m_doubleSpinBoxX->setRange(-999999.0, 999999.0);
		m_doubleSpinBoxY->setRange(-999999.0, 999999.0);
		m_doubleSpinBoxZ->setRange(-999999.0, 999999.0);

		// UIが変更されたら Apply を呼ぶ
		connect(m_doubleSpinBoxX, QOverload<double>::of(&StrictWheelDoubleSpinBox::valueChanged),
			[this](double val)
			{
				m_desc.apply(m_object, glm::vec3(val, m_doubleSpinBoxY->value(), m_doubleSpinBoxZ->value()), MyosotisFW::PropertyDesc::ChangeReason::UI_Preview);
				emit valueChanged(m_object, m_desc, MyosotisFW::PropertyDesc::ChangeReason::UI_Preview);
			});
		connect(m_doubleSpinBoxY, QOverload<double>::of(&StrictWheelDoubleSpinBox::valueChanged),
			[this](double val)
			{
				m_desc.apply(m_object, glm::vec3(m_doubleSpinBoxX->value(), val, m_doubleSpinBoxZ->value()), MyosotisFW::PropertyDesc::ChangeReason::UI_Preview);
				emit valueChanged(m_object, m_desc, MyosotisFW::PropertyDesc::ChangeReason::UI_Preview);
			});
		connect(m_doubleSpinBoxZ, QOverload<double>::of(&StrictWheelDoubleSpinBox::valueChanged),
			[this](double val)
			{
				m_desc.apply(m_object, glm::vec3(m_doubleSpinBoxX->value(), m_doubleSpinBoxY->value(), val), MyosotisFW::PropertyDesc::ChangeReason::UI_Preview);
				emit valueChanged(m_object, m_desc, MyosotisFW::PropertyDesc::ChangeReason::UI_Preview);
			});

		layout->addWidget(m_doubleSpinBoxX);
		layout->addWidget(m_doubleSpinBoxY);
		layout->addWidget(m_doubleSpinBoxZ);
		refreshUI();
	}

	void refreshUI() override
	{
		glm::vec3 val = std::get<glm::vec3>(m_desc.get(m_object));

		{// UI更新時に valueChanged シグナルが発生して apply が再送されないようにブロックする
			m_doubleSpinBoxX->blockSignals(true);
			m_doubleSpinBoxY->blockSignals(true);
			m_doubleSpinBoxZ->blockSignals(true);

			m_doubleSpinBoxX->setValue(val.x);
			m_doubleSpinBoxY->setValue(val.y);
			m_doubleSpinBoxZ->setValue(val.z);

			m_doubleSpinBoxX->blockSignals(false);
			m_doubleSpinBoxY->blockSignals(false);
			m_doubleSpinBoxZ->blockSignals(false);
		}
	}

private:
	StrictWheelDoubleSpinBox* m_doubleSpinBoxX;
	StrictWheelDoubleSpinBox* m_doubleSpinBoxY;
	StrictWheelDoubleSpinBox* m_doubleSpinBoxZ;
};