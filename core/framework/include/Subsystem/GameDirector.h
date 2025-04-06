// Copyright (c) 2025 kong9812
#pragma once
#include "ClassPointer.h"
#include "ObjectBase.h"
#include "RenderSubsystem.h"

namespace MyosotisFW::System::GameDirector {
	class GameDirector
	{
	public:
		GameDirector(const Render::RenderSubsystem_ptr& renderSubsystem);
		~GameDirector() {};

		void LoadGameStageFile(const std::string& fileName);
		void SaveGameStageFile(const std::string& fileName, const std::vector<ObjectBase_ptr>& objects);

	private:
		Render::RenderSubsystem_ptr m_renderSubsystem;
	};
	TYPEDEF_SHARED_PTR_ARGS(GameDirector)
}