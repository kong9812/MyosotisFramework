// Copyright (c) 2025 kong9812
#include <crtdbg.h>
#include <Windows.h>
#include <libloaderapi.h>
#include <errhandlingapi.h>
#include <iostream>
#include <filesystem>

#include "iqt.h"

int main(int argc, char* argv[])
{
	std::cout << std::filesystem::current_path() << std::endl;
	QApplication app(argc, argv);
	QMainWindow* mainWindow = new QMainWindow();
	mainWindow->show();
	int result = app.exec();
	delete mainWindow;
	return result;
}