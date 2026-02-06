// Copyright (c) 2025 kong9812
#pragma once
#include "ClassPointer.h"
#include <string>
#include <vector>

// 前方宣言
namespace MyosotisFW
{
	class MObject;
	TYPEDEF_SHARED_PTR_FWD(MObject);
	using MObjectList = std::vector<MObject_ptr>;
	using MObjectListPtr = std::shared_ptr<MObjectList>;
	using MObjectListConstPtr = std::shared_ptr<const MObjectList>;

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

		virtual void LoadMFWorld(const std::string& fileName);
		virtual void SaveMFWorld(const std::string& fileName, const MObjectListConstPtr& objects);

	protected:
		Render::RenderSubsystem_ptr m_renderSubsystem;
	};
	TYPEDEF_SHARED_PTR_ARGS(GameDirector);
}