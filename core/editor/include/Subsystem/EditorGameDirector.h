// Copyright (c) 2025 kong9812
#pragma once
#include "GameDirector.h"

// 前方宣言
namespace MyosotisFW
{
	class MObject;
	TYPEDEF_SHARED_PTR_FWD(MObject);
}

namespace MyosotisFW::System::GameDirector {
	class EditorGameDirector : public GameDirector
	{
	public:
		EditorGameDirector(const Render::RenderSubsystem_ptr& renderSubsystem) : GameDirector(renderSubsystem) {}
		~EditorGameDirector() {}

		MObject_ptr AddNewMObject();
	};
	TYPEDEF_SHARED_PTR_ARGS(EditorGameDirector);
}