// Copyright (c) 2025 kong9812
#pragma once
#include "classPointer.h"
#include "objectBase.h"
#include "renderSubsystem.h"

namespace MyosotisFW::System::GameDirector {
	class GameDirector
	{
	public:
		GameDirector(Render::RenderSubsystem_ptr& renderSubsystem);
		~GameDirector();

		ObjectBase_ptr CreateObject(ObjectType objectType);

		void LoadGameStageFile();
		void SaveGameStageFile();

	private:
		Render::RenderSubsystem_ptr m_renderSubsystem;
	};
	TYPEDEF_SHARED_PTR_ARGS(GameDirector)
}