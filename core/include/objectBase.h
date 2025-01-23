#pragma once
#include "vkStruct.h"

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
		ObjectBase() : m_transfrom({ glm::vec3(0.0f),glm::vec3(0.0f) ,glm::vec3(0.0f) }) {};
		virtual ~ObjectBase() = default;

		// todo.コンポーネント関連の処理

		// Update
		//virtual void Update(const Utility::Vulkan::Struct::UpdateData& updateData) = 0;
		// Render
		virtual void BindCommandBuffer(VkCommandBuffer commandBuffer) = 0;
		// Debug
		virtual void BindDebugGUIElement() = 0;

		const glm::vec3 GetPos() const { return m_transfrom.pos; }
		const glm::vec3 GetRot() const { return m_transfrom.rot; }
		const glm::vec3 GetScale() const { return m_transfrom.scale; }

	protected:
		Transform m_transfrom;
	};
}