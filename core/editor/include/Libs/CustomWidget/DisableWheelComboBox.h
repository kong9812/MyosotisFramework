// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"

// ホイールイベントを制限するカスタムComboBox
class DisableWheelComboBox : public QComboBox
{
public:
	explicit DisableWheelComboBox(QWidget* parent = nullptr) : QComboBox(parent), m_readOnly(false)
	{
		// クリックまたはTab移動でのみフォーカスを得る(ホイール回転でフォーカスを取れないようにする)
		setFocusPolicy(Qt::StrongFocus);
	}

	void SetReadOnly(bool readOnly)
	{
		m_readOnly = readOnly;
		if (m_readOnly)
		{
			setStyleSheet(
				"QComboBox { padding-right: 0px; }"
				"QComboBox::drop-down { width: 0px; border: none; }"
				"QComboBox::down-arrow { image: none; width: 0px; height: 0px; }"
				"QComboBox { background: transparent; border: none; padding: 2px; }"
			);
		}
	}
	bool IsReadOnly() const { return m_readOnly; }

protected:
	void wheelEvent(QWheelEvent* event) override
	{
		// ホイール入力を受け付けない
		event->ignore();
	}

	void showPopup() override
	{
		if (m_readOnly) return;
		QComboBox::showPopup();
	}

	void keyPressEvent(QKeyEvent* event) override
	{
		if (m_readOnly)
		{
			event->ignore();
			return;
		}
		QComboBox::keyPressEvent(event);
	}

private:
	bool m_readOnly;
};
