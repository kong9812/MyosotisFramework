// Copyright (c) 2025 kong9812
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "ClassPointer.h"
#include "DescriptorSetBase.h"
#include "CameraInfo.h"

namespace MyosotisFW::System::Render
{
	namespace Camera
	{
		class CameraBase;
		TYPEDEF_SHARED_PTR_FWD(CameraBase);
	}

	class SceneInfoDescriptorSet : public DescriptorSetBase
	{
	public:
		SceneInfoDescriptorSet(const RenderDevice_ptr& device, const VkDescriptorPool& descriptorPool);
		~SceneInfoDescriptorSet() {}

		enum class DescriptorBindingIndex : uint32_t
		{
			CameraInfo = 0,
			LightInfo,		// todo
			TerrainInfo,	// todo
			Count
		};

		void Update() override;
		void AddCamera(const Camera::CameraBase_ptr& camera);

	private:
		void updateCameraInfo();

		std::vector<Camera::CameraBase_ptr> m_cameras;
		CameraInfo m_cameraInfo;
	};

	TYPEDEF_SHARED_PTR_ARGS(SceneInfoDescriptorSet);
}