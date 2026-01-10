// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"
#include "ClassPointer.h"
#include "ComponentType.h"
#include "istduuid.h"
#include "PropertyEditorFactory.h"

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
	void UpdateView();

private:
	QWidget* m_mainWidget;
	QScrollArea* m_scrollArea;

	QLabel* m_name;
	QLabel* m_uuid;
	QLabel* m_dummy;
	QFormLayout* m_formLayout;
	QGroupBox* m_groupBox;

	QPushButton* m_addComponentButton;
	QMenu* m_addComponentMenu;
	QVBoxLayout* m_vBoxLayout;
	MyosotisFW::MObject* m_currentObject;

	std::unique_ptr<PropertyEditorFactory> m_propertyEditorFactory;
	std::vector<std::unique_ptr<PropertyEditorBase>> m_activeEditors;

private:
	void addComponent(const MyosotisFW::ComponentType type);
	void ClearFormLayout();

public slots:
	void setObject(MyosotisFW::MObject* object);

Q_SIGNALS:
	void sigAddComponent(const uuids::uuid& uuid, const MyosotisFW::ComponentType type);

};