// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"
#include "PropertyEditorBase.h"
#include "DisableWheelComboBox.h"

class EnumPropertyEditor : public PropertyEditorBase
{
	Q_OBJECT
public:
	EnumPropertyEditor(void* obj, const MyosotisFW::PropertyDesc& desc, QWidget* parent) :
		PropertyEditorBase(obj, desc, parent),
		m_comboBox(new DisableWheelComboBox(this))
	{
		QHBoxLayout* layout = new QHBoxLayout(this);
		layout->setContentsMargins(0, 0, 0, 0);

		// EnumItemをComboBoxに追加
		if (m_desc.enumItems)
		{
			for (size_t i = 0; i < m_desc.enumCount; ++i)
			{
				const MyosotisFW::EnumItem& item = m_desc.enumItems[i];
				m_comboBox->addItem(item.name, item.value);
			}
		}

		// 変更されたらApplyを呼ぶ
		connect(m_comboBox, QOverload<int>::of(&DisableWheelComboBox::currentIndexChanged),
			[this](int index)
			{
				if (index < 0) return;

				// ComboBoxデータ(int32_t)を取得
				int32_t val = static_cast<int32_t>(m_comboBox->currentData().toInt());

				// applyを実行
				if (m_desc.apply)
				{
					m_desc.apply(m_object, val, MyosotisFW::ChangeReason::UI_Preview);
				}
			});

		layout->addWidget(m_comboBox);
		refreshUI();
	}

	void refreshUI() override
	{
		if (!m_desc.get) return;

		// 現在の値
		MyosotisFW::PropertyValue currentVal = m_desc.get(m_object);
		int32_t val = std::get<int32_t>(currentVal);

		// ComboBoxから該当する値を持つインデックスを探す
		int index = m_comboBox->findData(val);
		if (index != -1)
		{
			{// UI更新時に valueChanged シグナルが発生して apply が再送されないようにブロックする
				m_comboBox->blockSignals(true);
				m_comboBox->setCurrentIndex(index);
				m_comboBox->blockSignals(false);
			}
		}
	}

private:
	DisableWheelComboBox* m_comboBox;

};