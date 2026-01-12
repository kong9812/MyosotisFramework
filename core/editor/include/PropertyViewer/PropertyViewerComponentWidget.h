// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"
#include "ComponentBase.h"
#include "PropertyEditorFactory.h"

class PropertyViewerComponentWidget : public QWidget {
	Q_OBJECT
public:
	PropertyViewerComponentWidget(const QString& title, int animationDuration = 300, QWidget* parent = nullptr)
		: QWidget(parent),
		m_mainWidget(new QWidget(this)),
		m_container(new QWidget(this)),
		m_toolButton(new QToolButton(m_mainWidget)),
		m_scrollArea(new QScrollArea(m_mainWidget)),
		m_vBoxLayout(new QVBoxLayout(this)),
		m_formLayout(new QFormLayout(m_container)),
		m_propertyAnimation(new QPropertyAnimation(this)),
		m_propertyEditorFactory(std::make_unique<PropertyEditorFactory>()),
		m_activeEditors()
	{
		m_toolButton->setStyleSheet("QToolButton { border: none; font-weight: bold; }");
		m_toolButton->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
		m_toolButton->setArrowType(Qt::ArrowType::DownArrow);
		m_toolButton->setText(title);
		m_toolButton->setCheckable(true);
		m_toolButton->setChecked(true);

		m_scrollArea->setStyleSheet("QScrollArea { border: none; }");
		m_scrollArea->setMaximumHeight(0);
		m_scrollArea->setMinimumHeight(0);
		m_scrollArea->setWidgetResizable(true);

		m_propertyAnimation->setPropertyName("maximumHeight");
		m_propertyAnimation->setTargetObject(m_scrollArea);
		m_propertyAnimation->setDuration(animationDuration);

		m_vBoxLayout->setAlignment(Qt::AlignmentFlag::AlignTop);
		m_vBoxLayout->setContentsMargins(0, 0, 0, 0);
		m_vBoxLayout->setSpacing(0);
		m_vBoxLayout->addWidget(m_toolButton);
		m_vBoxLayout->addWidget(m_scrollArea);
		m_vBoxLayout->addWidget(m_container);

		connect(m_toolButton, &QToolButton::toggled, this, &PropertyViewerComponentWidget::toggle);
	}

	void SetComponent(MyosotisFW::ComponentBase_ptr component)
	{
		// property 表示
		MyosotisFW::PropertyTable propertyTable = component->GetPropertyTable();
		propertyTable.ForEach([&](const MyosotisFW::PropertyDesc& desc)
			{
				//components += std::string("ID: ") + uuids::to_string(desc.id) + "\n";
				//components += std::string("type: ") + desc.type.name + "(" + uuids::to_string(desc.type.id) + ")" + "\n";
				//components += std::string("category: ") + desc.category + "\n";
				//components += std::string("name: ") + desc.name + "\n";
				//components += "\n";
				std::unique_ptr<PropertyEditorBase> editorWidget = m_propertyEditorFactory->CreateEditor(component.get(), desc, m_container);
				if (editorWidget)
				{
					PropertyEditorBase* ptr = editorWidget.get();
					m_activeEditors.emplace_back(std::move(editorWidget));
					m_formLayout->addRow(desc.name, ptr);
				}
			});
	}

private slots:
	void toggle(bool expanded)
	{
		m_container->setVisible(expanded);
		m_toolButton->setArrowType(expanded ? Qt::ArrowType::DownArrow : Qt::ArrowType::RightArrow);
		m_propertyAnimation->setDirection(expanded ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
		m_propertyAnimation->start();
	}

private:
	QWidget* m_mainWidget;
	QWidget* m_container;
	QToolButton* m_toolButton;
	QScrollArea* m_scrollArea;
	QVBoxLayout* m_vBoxLayout;
	QPropertyAnimation* m_propertyAnimation;

	QFormLayout* m_formLayout;
	std::unique_ptr<PropertyEditorFactory> m_propertyEditorFactory;
	std::vector<std::unique_ptr<PropertyEditorBase>> m_activeEditors;
};