// Copyright (c) 2025 kong9812
#pragma once
#include <string>

#include "ComponentCast.h"
#include "iglm.h"
#include "ComponentBase.h"
#include "ClassPointer.h"
#include "CameraData.h"
#include "UpdateData.h"
#include "Ray.h"

namespace MyosotisFW::System::Render::Camera
{
	class CameraBase : public ComponentBase
	{
	public:
		CameraBase(const uint32_t objectID);
		virtual ~CameraBase() = default;

		void ResetCamera();
		virtual glm::mat4 GetViewMatrix() const;
		virtual glm::mat4 GetProjectionMatrix() const;
		virtual glm::vec3 GetWorldPos(const glm::vec2& pos, const float distance) const;
		CameraData GetCameraData() const;
		virtual const ComponentType GetType() const override { return ComponentType::Undefined; }

		void UpdateScreenSize(const glm::vec2& size);

		float GetDistance(const glm::vec3& pos) const;
		glm::vec3 GetFrontPos(const float distance) const;
		Ray GetRay(const glm::vec2& pos) const;

		glm::vec3 GetCameraPos() const { return m_cameraPos; }

		void SetMainCamera(bool isMainCamera) { m_isMainCamera = isMainCamera; }
		bool IsMainCamera() const { return m_isMainCamera; }

		virtual void Update(const UpdateData& updateData) {}

		bool IsMoved() const { return m_isMoved; }

		virtual rapidjson::Value Serialize(rapidjson::Document::AllocatorType& allocator) const override;
		virtual void Deserialize(const rapidjson::Value& doc) override;
	protected:
		virtual void initialize();

		bool m_isMainCamera;
		bool m_isMoved;

		// カメラ位置
		glm::vec3 m_cameraPos;
		// カメラ視点
		glm::vec3 m_cameraLookAt;
		// カメラ　前ベクトル
		glm::vec3 m_cameraFront;
		// カメラ　上ベクトル
		glm::vec3 m_cameraUp;
		// カメラ　右ベクトル
		glm::vec3 m_cameraRight;
		// カメラ FOV
		float m_cameraFov;
		// カメラ Far
		float m_cameraFar;
		// カメラ Near
		float m_cameraNear;
		// アスペクト比
		float m_aspectRadio;
		// 画面サイズ
		glm::vec2 m_screenSize;

	public:
		// ComponentProperty
		static const PropertyTable& StaticPropertyTable()
		{
			static const PropertyDesc props[] = {
				MakeProperty<CameraBase, glm::vec3, &CameraBase::m_cameraPos>(uuids::hashMaker(), "Position", "Camera"),
				MakeProperty<CameraBase, float, &CameraBase::m_cameraFov>(uuids::hashMaker(), "FOV", "Camera"),
				MakeProperty<CameraBase, float, &CameraBase::m_cameraFar>(uuids::hashMaker(), "Far", "Camera"),
				MakeProperty<CameraBase, float, &CameraBase::m_cameraNear>(uuids::hashMaker(), "Near", "Camera"),
				MakeProperty<CameraBase, glm::vec3, &CameraBase::m_cameraLookAt>(uuids::hashMaker(), "LookAt", "Camera", PropertyDesc::PropertyFlags::ReadOnly),
				MakeProperty<CameraBase, glm::vec3, &CameraBase::m_cameraFront>(uuids::hashMaker(), "Front", "Camera", PropertyDesc::PropertyFlags::ReadOnly),
				MakeProperty<CameraBase, glm::vec3, &CameraBase::m_cameraUp>(uuids::hashMaker(), "Up", "Camera", PropertyDesc::PropertyFlags::ReadOnly),
				MakeProperty<CameraBase, glm::vec3, &CameraBase::m_cameraRight>(uuids::hashMaker(), "Right", "Camera", PropertyDesc::PropertyFlags::ReadOnly),
				MakeProperty<CameraBase, float, &CameraBase::m_aspectRadio>(uuids::hashMaker(), "Radio", "Camera", PropertyDesc::PropertyFlags::ReadOnly),
				MakeProperty<CameraBase, glm::vec2, &CameraBase::m_screenSize>(uuids::hashMaker(), "ScreenSize", "Camera", PropertyDesc::PropertyFlags::ReadOnly,
					+[](void* obj, const PropertyDesc::PropertyValue& v, PropertyDesc::ChangeReason cr)
					{
						static_cast<CameraBase*>(obj)->UpdateScreenSize(std::get<glm::vec2>(v));
					}),
			};
			static const PropertyTable table{ &ComponentBase::StaticPropertyTable(), props, std::size(props) };
			return table;
		}
		const PropertyTable& GetPropertyTable() const override { return StaticPropertyTable(); }

	protected:
		virtual void OnPropertyChanged(uuids::uuid propertyID) {}

	};
	TYPEDEF_SHARED_PTR_ARGS(CameraBase);
	OBJECT_CAST_FUNCTION(CameraBase);
}