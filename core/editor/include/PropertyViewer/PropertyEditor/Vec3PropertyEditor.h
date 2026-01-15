// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"
#include "PropertyEditorBase.h"

class Vec3PropertyEditor : public PropertyEditorBase
{
public:
	Vec3PropertyEditor(void* obj, const MyosotisFW::PropertyDesc& desc, QWidget* parent) :
		PropertyEditorBase(obj, desc, parent),
		m_doubleSpinBoxX(new QDoubleSpinBox(this)),
		m_doubleSpinBoxY(new QDoubleSpinBox(this)),
		m_doubleSpinBoxZ(new QDoubleSpinBox(this))
	{
		QHBoxLayout* layout = new QHBoxLayout(this);
		layout->setContentsMargins(0, 0, 0, 0);

		m_doubleSpinBoxX->setRange(-999999.0, 999999.0);
		m_doubleSpinBoxY->setRange(-999999.0, 999999.0);
		m_doubleSpinBoxZ->setRange(-999999.0, 999999.0);

		// UIが変更されたら Apply を呼ぶ
		connect(m_doubleSpinBoxX, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
			[this](double val)
			{
				m_desc.apply(m_object, glm::vec3(val, m_doubleSpinBoxY->value(), m_doubleSpinBoxZ->value()), MyosotisFW::ChangeReason::UI_Preview);
			});
		connect(m_doubleSpinBoxY, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
			[this](double val)
			{
				m_desc.apply(m_object, glm::vec3(m_doubleSpinBoxX->value(), val, m_doubleSpinBoxZ->value()), MyosotisFW::ChangeReason::UI_Preview);
			});
		connect(m_doubleSpinBoxZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
			[this](double val)
			{
				m_desc.apply(m_object, glm::vec3(m_doubleSpinBoxX->value(), m_doubleSpinBoxY->value(), val), MyosotisFW::ChangeReason::UI_Preview);
			});

		layout->addWidget(m_doubleSpinBoxX);
		layout->addWidget(m_doubleSpinBoxY);
		layout->addWidget(m_doubleSpinBoxZ);
		refreshUI();
	}

	void refreshUI() override
	{
		glm::vec3 val = std::get<glm::vec3>(m_desc.get(m_object));
		m_doubleSpinBoxX->setValue(val.x);
		m_doubleSpinBoxY->setValue(val.y);
		m_doubleSpinBoxZ->setValue(val.z);
	}

private:
	QDoubleSpinBox* m_doubleSpinBoxX;
	QDoubleSpinBox* m_doubleSpinBoxY;
	QDoubleSpinBox* m_doubleSpinBoxZ;
};