// Copyright (c) 2025 kong9812
#pragma once
#include <functional>
#include "GameDirector.h"
#include "ComponentType.h"

// 前方宣言
namespace MyosotisFW
{
	class MObject;
	TYPEDEF_SHARED_PTR_FWD(MObject);
	class ComponentBase;
	TYPEDEF_SHARED_PTR_FWD(ComponentBase);
}

namespace MyosotisFW::System::GameDirector {
	class EditorGameDirector : public GameDirector
	{
	public:
		EditorGameDirector(const Render::RenderSubsystem_ptr& renderSubsystem) : GameDirector(renderSubsystem), m_mfWorldLoadedCallback(nullptr) {}
		~EditorGameDirector() {}

		void LoadMFWorld(const std::string& fileName) override;

		MObject_ptr AddNewMObject();
		ComponentBase_ptr RegisterComponent(const uuids::uuid& uuid, const MyosotisFW::ComponentType type);
		void SetMeshChangedCallback(const std::function<void(std::vector<MObject_ptr>)>& callback) { m_mfWorldLoadedCallback = callback; }

	private:
		std::function<void(std::vector<MObject_ptr>)> m_mfWorldLoadedCallback;
	};
	TYPEDEF_SHARED_PTR_ARGS(EditorGameDirector);
}