// Copyright (c) 2025 kong9812
#pragma once

#ifdef DLL_EXPORTS
#define DLL_CLASS __declspec(dllexport)
#else
#define DLL_CLASS __declspec(dllimport)
#endif

class DLL_CLASS IApplication
{
public:
	virtual ~IApplication() = default;
	virtual int Run() = 0;

private:

};