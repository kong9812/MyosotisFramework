// Copyright (c) 2025 kong9812
#pragma once
#include <vector>
#include <list>
#include <unordered_map>
#include "ClassPointer.h"
#include "AccelerationStructure.h"
#include "Mesh.h"
#include "BLASInfo.h"
#include "TLASInfo.h"
#include "TLASInstance.h"

namespace MyosotisFW
{
	// 前方宣言
	class MObject;
	TYPEDEF_SHARED_PTR_FWD(MObject);
}

namespace MyosotisFW::System::Render
{
	// 前方宣言
	class RenderDevice;
	TYPEDEF_SHARED_PTR_FWD(RenderDevice);
	class RenderDescriptors;
	TYPEDEF_SHARED_PTR_FWD(RenderDescriptors);
	class RenderResources;
	TYPEDEF_SHARED_PTR_FWD(RenderResources);

	class AccelerationStructureManager
	{
	public:
		AccelerationStructureManager(
			const RenderDevice_ptr& device,
			const RenderDescriptors_ptr& renderDescriptors,
			const RenderResources_ptr& renderResources);
		~AccelerationStructureManager();

		void OnLoadedMesh(MeshesHandle& meshes);
		// todo. Meshに対応するBLASを登録し、ビルドが必要な状態にする
		// todo. MeshデータからBLASを生成するためのリソースを作成し、
		// todo. Vertex/IndexのデバイスアドレスとBLASを登録する

		void OnAddObject(const MObject_ptr& object);		// todo. TLAS用のInstance情報を追加し、TLAS更新を要求する

		void RebuildTLAS();			// todo. シーン切り替え時にTLASとInstance情報をリセットする

		void Process();			//todo. ここでビルドと更新を判定し、実行する

	private:
		RenderDevice_ptr m_device;
		RenderDescriptors_ptr m_renderDescriptors;
		RenderResources_ptr m_renderResources;

		bool m_blasDirty;
		bool m_tlasDirty;

		std::vector<BLASInfo> m_blas;
		TLASInfo m_tlas;

		std::vector<BLASInfo> m_pendingBLASBuild;
		std::list<TLASInstance_ptr> m_instances;

		void buildBLAS();	// todo. 登録済みBLASをGPU上にBuildする（初回のみ）
		void buildTLAS();	// todo. TLASのビルド処理
		void updateTLAS() {} // todo. TLASの更新処理
		void destroyTLAS();

	private:
		PFN_vkGetRayTracingShaderGroupHandlesKHR		m_vkGetRayTracingShaderGroupHandlesKHR;
		PFN_vkCreateAccelerationStructureKHR			m_vkCreateAccelerationStructureKHR;
		PFN_vkCmdBuildAccelerationStructuresKHR			m_vkCmdBuildAccelerationStructuresKHR;
		PFN_vkGetAccelerationStructureDeviceAddressKHR	m_vkGetAccelerationStructureDeviceAddressKHR;
		PFN_vkGetAccelerationStructureBuildSizesKHR		m_vkGetAccelerationStructureBuildSizesKHR;
		PFN_vkCmdTraceRaysKHR							m_vkCmdTraceRaysKHR;
		PFN_vkCreateRayTracingPipelinesKHR				m_vkCreateRayTracingPipelinesKHR;
		PFN_vkDestroyAccelerationStructureKHR			m_vkDestroyAccelerationStructureKHR;

		struct ScratchBuffer
		{
			Buffer buffer;
			VkDeviceAddress deviceAddress;
		};
	};
	TYPEDEF_SHARED_PTR_ARGS(AccelerationStructureManager);
}