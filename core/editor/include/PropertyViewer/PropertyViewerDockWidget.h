// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"
#include "ClassPointer.h"

// 前方宣言
namespace MyosotisFW
{
	class MObject;
	TYPEDEF_SHARED_PTR_FWD(MObject);
}

class PropertyViewerDockWidget : public QDockWidget
{
	Q_OBJECT
public:
	PropertyViewerDockWidget(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

private:
	QWidget* m_mainWidget;

	QLabel* m_name;

	QVBoxLayout* m_vBoxLayout;

public slots:
	void setObject(MyosotisFW::MObject* object);

};