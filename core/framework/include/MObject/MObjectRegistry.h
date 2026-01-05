// Copyright (c) 2025 kong9812
#pragma once
#include <vector>
#include <deque>
#include <functional>
#include <unordered_map>
#include "ClassPointer.h"
#include "ObjectInfo.h"
#include "ComponentType.h"
#include "istduuid.h"

namespace MyosotisFW
{
	class MObject;
	TYPEDEF_SHARED_PTR_FWD(MObject);
	using MObjectList = std::vector<MObject_ptr>;
	using MObjectListPtr = std::shared_ptr<MObjectList>;
	using MObjectListConstPtr = std::shared_ptr<const MObjectList>;
	using MObjectMap = std::unordered_map<uuids::uuid, MObject_ptr>;	// 内部専用

	class ComponentBase;
	TYPEDEF_SHARED_PTR_FWD(ComponentBase);

	namespace System::Render
	{
		class RenderDevice;
		TYPEDEF_SHARED_PTR_FWD(RenderDevice);
		class RenderResources;
		TYPEDEF_SHARED_PTR_FWD(RenderResources);
		class RenderDescriptors;
		TYPEDEF_SHARED_PTR_FWD(RenderDescriptors);
	}

	class MObjectRegistry
	{
	public:
		MObjectRegistry(const System::Render::RenderDevice_ptr& device, const System::Render::RenderResources_ptr& resources, const System::Render::RenderDescriptors_ptr& descriptors);
		~MObjectRegistry() {}

		MObject_ptr CreateNewObject();
		ComponentBase_ptr RegisterComponent(const uuids::uuid& uuid, const ComponentType type);
		MObjectListConstPtr GetMObjectList() const { return m_objects; }

		void Clear();

		const ObjectInfo& GetObjectInfo(const size_t index) const;
		std::vector<ObjectInfo> GetAllObjectInfoCopy() const;

		bool IsMeshChanged() const { return m_meshChanged; }
		bool IsTransformChanged() const { return m_transformChanged; }

		void MeshChanged() { m_meshChanged = true; }
		void TransformChanged() { m_transformChanged = true; }
		void ResetChangeFlags() { m_meshChanged = false; m_transformChanged = false; }

		void SetOnAddObject(const std::function<void(const MObject_ptr&)> callback) { m_onAddObject = callback; }

	private:
		System::Render::RenderDevice_ptr m_device;
		System::Render::RenderResources_ptr m_resources;
		System::Render::RenderDescriptors_ptr m_renderDescriptors;

		MObjectListPtr m_objects;
		MObjectMap m_objectMap;	// 内部専用

		std::deque<ObjectInfo> m_objectInfo;

		bool m_meshChanged;
		bool m_transformChanged;

		std::function<void(const MObject_ptr&)> m_onAddObject;
	};
	TYPEDEF_SHARED_PTR_ARGS(MObjectRegistry);
};