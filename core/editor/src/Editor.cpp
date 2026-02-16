// Copyright (c) 2025 kong9812
#define DLL_EXPORTS
#include "Editor.h"

void Editor::Initialize(const bool& allowHotReload)
{
#ifdef _MSC_VER
	// メモリリークチェッカ
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	m_allowHotReload = allowHotReload;
}

int Editor::Run()
{
	m_mainWindow->show();
	return m_application.exec();
}