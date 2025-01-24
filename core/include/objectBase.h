#pragma once
#include "classPointer.h"
#include "objectType.h"
#include "vkStruct.h"
#include "istduuid.h"

namespace MyosotisFW 
{
	typedef struct
	{
		glm::vec3 pos;
		glm::vec3 rot;
		glm::vec3 scale;
	}Transform;

	class ObjectBase
	{
	public:
		ObjectBase(ObjectType objectType) :
			m_transfrom({ glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f) }),
			m_objectType(objectType),
			m_objectID(),
			m_readyForRender(false)
		{
			m_typeID = g_objectTypeUUID.at(objectType).value();
		};

		virtual ~ObjectBase() = default;

		// todo.コンポーネント関連の処理

		// Update
		//virtual void Update(const Utility::Vulkan::Struct::UpdateData& updateData) = 0;
		
		// Render
		virtual void BindCommandBuffer(VkCommandBuffer commandBuffer) = 0;
		// Debug
		virtual void BindDebugGUIElement() = 0;

		const ObjectType GetObjectType() const { return m_objectType; }
		const uuids::uuid GetTypeID() const { return m_typeID; }
		const uuids::uuid GetObjectID() const { return m_objectID; }

		const glm::vec3 GetPos() const { return m_transfrom.pos; }
		const glm::vec3 GetRot() const { return m_transfrom.rot; }
		const glm::vec3 GetScale() const { return m_transfrom.scale; }

	protected:
		bool m_readyForRender;
		uuids::uuid m_typeID;
		uuids::uuid m_objectID;

		Transform m_transfrom;
		ObjectType m_objectType;
	};
	TYPEDEF_SHARED_PTR_ARGS(ObjectBase)
};