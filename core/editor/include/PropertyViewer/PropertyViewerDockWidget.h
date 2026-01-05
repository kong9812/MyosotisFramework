// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"
#include "ClassPointer.h"
#include "ComponentType.h"
#include "istduuid.h"

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
	QLabel* m_uuid;

	QPushButton* m_addComponentButton;
	QMenu* m_addComponentMenu;

	QVBoxLayout* m_vBoxLayout;
	MyosotisFW::MObject* m_currentObject;

public slots:
	void setObject(MyosotisFW::MObject* object);

Q_SIGNALS:
	void addComponent(const uuids::uuid& uuid, const MyosotisFW::ComponentType type);

};