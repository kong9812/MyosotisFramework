// Copyright (c) 2025 kong9812
#ifdef _WIN32
#include <crtdbg.h>
#endif
#include <iostream>
#include <filesystem>

#include "iqt.h"
#include "Logger.h"
#include "Editor.h"

int main(int argc, char* argv[])
{
#ifdef DEBUG
	Logger::ClearLog();
#endif
	Logger::Info(std::filesystem::current_path().string());
	Editor* editor = new Editor(argc, argv);
	editor->Initialize(false);
	int result = editor->Run();
	delete editor;
	return result;
}