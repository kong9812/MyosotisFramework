// Copyright (c) 2025 kong9812
#pragma once
#include "ClassPointer.h"
#include <string>
#include <vector>

// 前方宣言
namespace MyosotisFW
{
	class ComponentBase;
	TYPEDEF_SHARED_PTR_FWD(ComponentBase);
	using ComponentBaseList = std::vector<ComponentBase_ptr>;
	using ComponentBaseListPtr = std::shared_ptr<ComponentBaseList>;

	namespace System::Render
	{
		class RenderSubsystem;
		TYPEDEF_SHARED_PTR_FWD(RenderSubsystem);
	}
}

namespace MyosotisFW::System::GameDirector {
	class GameDirector
	{
	public:
		GameDirector(const Render::RenderSubsystem_ptr& renderSubsystem);
		~GameDirector() {}

		void LoadGameStageFile(const std::string& fileName);
		void SaveGameStageFile(const std::string& fileName, const ComponentBaseListPtr& objects);

	protected:
		Render::RenderSubsystem_ptr m_renderSubsystem;
	};
	TYPEDEF_SHARED_PTR_ARGS(GameDirector);
}