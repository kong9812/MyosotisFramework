// Copyright (c) 2025 kong9812
#pragma once

class IApplication
{
public:
	virtual ~IApplication() = default;
	virtual void Initialize(const bool& allowHotReload) = 0;
	virtual int Run() = 0;
};