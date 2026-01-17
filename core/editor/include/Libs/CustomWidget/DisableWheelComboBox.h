// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"

// ホイールイベントを制限するカスタムComboBox
class DisableWheelComboBox : public QComboBox
{
public:
	explicit DisableWheelComboBox(QWidget* parent = nullptr) : QComboBox(parent)
	{
		// クリックまたはTab移動でのみフォーカスを得る(ホイール回転でフォーカスを取れないようにする)
		setFocusPolicy(Qt::StrongFocus);
	}

protected:
	void wheelEvent(QWheelEvent* event) override
	{
		// ホイール入力を受け付けない
		event->ignore();
	}
};
