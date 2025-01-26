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
		~GameDirector() {};

		void LoadGameStageFile(std::string fileName);
		void SaveGameStageFile(std::string fileName, const std::vector<ObjectBase_ptr>& objects);

	private:
		Render::RenderSubsystem_ptr m_renderSubsystem;
	};
	TYPEDEF_SHARED_PTR_ARGS(GameDirector)
}