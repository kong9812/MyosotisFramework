// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"
#include "PropertyEditorBase.h"

class StringPropertyEditor : public PropertyEditorBase
{
public:
	StringPropertyEditor(void* obj,
		const MyosotisFW::PropertyDesc& desc,
		QWidget* parent) :
		PropertyEditorBase(obj, desc, parent),
		m_lineEdit(new QLineEdit(this))
	{
		QHBoxLayout* layout = new QHBoxLayout(this);
		layout->setContentsMargins(0, 0, 0, 0);

		// 読み取り専用の対応
		if (MyosotisFW::HasPropertyFlag(desc.flags, MyosotisFW::PropertyDesc::PropertyFlags::ReadOnly))
		{
			m_lineEdit->setReadOnly(true);
			m_lineEdit->setStyleSheet(
				"QLineEdit {"
				"  background: transparent;"
				"  border: none;"
				"  padding: 2px;"
				"}"
			);
		}

		m_lineEdit->setPlaceholderText(tr("Enter text..."));

		connect(m_lineEdit, &QLineEdit::textChanged, [this](const QString& text)
			{
				m_desc.apply(m_object, text.toStdString(), MyosotisFW::PropertyDesc::ChangeReason::UI_Preview);
				emit valueChanged(m_object, m_desc, MyosotisFW::PropertyDesc::ChangeReason::UI_Preview);
			});

		connect(m_lineEdit, &QLineEdit::editingFinished, [this]()
			{
				if (m_lineEdit->isModified())
				{
					m_desc.apply(m_object, m_lineEdit->text().toStdString(), MyosotisFW::PropertyDesc::ChangeReason::UI_Commit);
					m_lineEdit->setModified(false);
					emit valueChanged(m_object, m_desc, MyosotisFW::PropertyDesc::ChangeReason::UI_Commit);
				}
			});

		layout->addWidget(m_lineEdit);
		refreshUI();
	}

	void refreshUI() override
	{
		MyosotisFW::PropertyDesc::PropertyValue valVariant = m_desc.get(m_object);
		if (std::holds_alternative<std::string>(valVariant))
		{
			QString text = QString::fromStdString(std::get<std::string>(valVariant));

			{// UI更新時に valueChanged シグナルが発生して apply が再送されないようにブロックする
				m_lineEdit->blockSignals(true);
				if (m_lineEdit->text() != text)
				{
					m_lineEdit->setText(text);
				}
				m_lineEdit->setModified(false);
				m_lineEdit->blockSignals(false);
			}
		}
	}

private:
	QLineEdit* m_lineEdit;
};