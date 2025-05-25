// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"
#include "MainWindow.h"

class Editor
{
public:
	Editor(int argc, char* argv[]) :
		m_application(argc, argv),
		m_mainWindow(new MyosotisFW::System::Editor::MainWindow()),
		m_allowHotReload(false) {
	}
	~Editor() {}

	void Initialize(const bool& allowHotReload);
	int Run();

private:
	QApplication m_application;
	QScopedPointer<MyosotisFW::System::Editor::MainWindow> m_mainWindow;
	bool m_allowHotReload;
};