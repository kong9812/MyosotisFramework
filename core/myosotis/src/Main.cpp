// Copyright (c) 2025 kong9812
#include <crtdbg.h>
#include <Windows.h>
#include <libloaderapi.h>
#include <errhandlingapi.h>
#include <iostream>
#include <filesystem>

#include "Application.h"
#include "ApplicationInterface.h"

#ifdef FWDLL
namespace {
	constexpr char* g_dll = "MyosotisDLL.dll";
	constexpr char* g_dllBuilder = "tools\\vs22BuildMyosotisDLL_debug.bat";
	typedef IApplication* (*GetInstanceFunction)();
}

IApplication* GetApplication(HMODULE hModule)
{
	GetInstanceFunction function = reinterpret_cast<GetInstanceFunction>(GetProcAddress(hModule, "GetInstance"));
	return function();
}

HMODULE GetDllModule()
{
	HMODULE hModule = LoadLibrary(g_dll);
	if (!hModule) {
		std::cerr << "Failed to load DLL: " << GetLastError() << '\n';
		return NULL;
	}
	return hModule;
}

bool BuildDLL(bool force = false)
{
	// あればビルドしなくてもいい
	if ((std::filesystem::exists(g_dll)) && (!force)) return true;

	// なければビルドする (Visual Studio環境を想定する)
	if (!std::filesystem::exists(g_dllBuilder)) return false;

	return std::system(g_dllBuilder) == EXIT_SUCCESS;
}

int main()
{
	std::cout << std::filesystem::current_path() << std::endl;

	// メモリリークチェッカ
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// DLLの準備
	if (!BuildDLL()) return 1;

	HMODULE hModule = GetDllModule();
	if (!hModule) return 1;

	IApplication* application = GetApplication(hModule);
	application->Initialize(true);
	while (true)
	{
		int result = application->Run();
		// Hot Reload
		if (result == 2)
		{
			delete application;
			FreeLibrary(hModule);

			std::filesystem::rename("MyosotisDLL.pdb", "MyosotisDLL_.pdb");
			if (!BuildDLL(true)) return 1;
			std::filesystem::remove("MyosotisDLL_.pdb");

			hModule = GetDllModule();
			if (!hModule) return 1;
			application = GetApplication(hModule);
			application->Initialize(true);
			continue;
		}
		break;
	}

	delete application;
	FreeLibrary(hModule);
}
#else
int main()
{
	std::cout << std::filesystem::current_path() << std::endl;
	Application* application = new Application();
	application->Initialize(false);
	int result = application->Run();
	delete application;
	return result;
}
#endif