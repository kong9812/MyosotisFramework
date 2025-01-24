// Copyright (c) 2025 kong9812
#include "gameDirector.h"
#include "logger.h"
#include "istduuid.h"
#include "fpsCamera.h"
#include "primitiveGeometry.h"

namespace MyosotisFW::System::GameDirector {
	GameDirector::GameDirector(Render::RenderSubsystem_ptr& renderSubsystem)
	{
		m_renderSubsystem = renderSubsystem;

		// Test
		ObjectBase_ptr test = CreateObject(ObjectType::StaticMesh);
		m_renderSubsystem->ResistObject(test);
	}

	GameDirector::~GameDirector()
	{

	}

	ObjectBase_ptr GameDirector::CreateObject(ObjectType objectType)
	{
		ASSERT((objectType != ObjectType::Undefined) || (objectType != ObjectType::Max), "Error type");

		ObjectBase_ptr object{};

		// todo. factoryを用意
		switch (objectType)
		{
		case ObjectType::Camera:
		{
			object = Render::Camera::CreateFPSCameraPointer();
		}
		break;
		case ObjectType::StaticMesh:
		{
			object = Render::CreatePrimitiveGeometryPointer();
		}
		break;
		default:
			break;
		}
		return object;
	}

	void GameDirector::LoadGameStageFile()
	{
		Logger::Info("LoadGameStageFile");

	}

	void GameDirector::SaveGameStageFile()
	{
		Logger::Info("SaveGameStageFile");

	}
}
