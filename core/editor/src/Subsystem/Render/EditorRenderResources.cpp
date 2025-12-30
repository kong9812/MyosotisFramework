// Copyright (c) 2025 kong9812
#include "EditorRenderResources.h"

#include "RenderDevice.h"

#include "VK_CreateInfo.h"
#include "AppInfo.h"

namespace MyosotisFW::System::Render
{
	EditorRenderResources::~EditorRenderResources()
	{

	}

	void EditorRenderResources::Initialize(const glm::ivec2& screenSize)
	{
		__super::Initialize(screenSize);
	}

	void EditorRenderResources::Resize(const glm::ivec2& screenSize)
	{
		__super::Resize(screenSize);
	}
}