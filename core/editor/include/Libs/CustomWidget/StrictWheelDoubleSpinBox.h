// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"

// ホイールイベントを制限するカスタムSpinBox
class StrictWheelDoubleSpinBox : public QDoubleSpinBox
{
public:
	explicit StrictWheelDoubleSpinBox(QWidget* parent = nullptr) : QDoubleSpinBox(parent)
	{
		// クリックまたはTab移動でのみフォーカスを得る(ホイール回転でフォーカスを取れないようにする)
		setFocusPolicy(Qt::StrongFocus);
	}

protected:
	void wheelEvent(QWheelEvent* event) override
	{
		// フォーカスがある(クリックして選択されている)時だけホイール入力を受け付ける
		if (hasFocus())
		{
			QDoubleSpinBox::wheelEvent(event);
		}
		else
		{
			event->ignore();
		}
	}
};
