// Copyright (c) 2025 kong9812
#include "EditorCamera.h"
#include "iglfw.h"
#include "AppInfo.h"

#include "iimgui.h"

namespace MyosotisFW::System::Render::Camera
{
	EditorCamera::EditorCamera(const uint32_t objectID) : CameraBase(objectID)
	{
		m_lastMousePos = glm::vec3(0.0f);
		m_name = "EditorCamera";
	}

	glm::mat4 EditorCamera::GetViewMatrix() const
	{
		return glm::lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);
	}

	glm::mat4 EditorCamera::GetProjectionMatrix() const
	{
		return glm::perspective(glm::radians(m_cameraFov), m_aspectRadio, m_cameraNear, m_cameraFar);
	}

	glm::vec3 EditorCamera::GetWorldPos(const glm::vec2& pos, const float distance) const
	{
		return __super::GetWorldPos(pos, distance);
	}

	void EditorCamera::Update(const UpdateData& updateData)
	{
		if (updateData.pause) return;

		if (!m_isReady)
		{
			// 最初の位置をリセット
			m_lastMousePos = updateData.mousePos;
			m_isReady = true;
		}

		bool allowMouseMovement = false;
		{// mouse movement (Left ALT && Mouse Left Button)
			auto key = updateData.keyActions.find(GLFW_KEY_LEFT_ALT);
			auto mouseBtn = updateData.mouseButtonActions.find(GLFW_MOUSE_BUTTON_LEFT);
			if (key != updateData.keyActions.end() && mouseBtn != updateData.mouseButtonActions.end())
			{
				if (((key->second == GLFW_PRESS) || (key->second == GLFW_REPEAT)) &&
					((mouseBtn->second == GLFW_PRESS) || (mouseBtn->second == GLFW_REPEAT)))
				{
					allowMouseMovement = true;
				}
			}
		}

		if (allowMouseMovement)
		{
			glm::vec2 mouseMovement{};
			mouseMovement.x = m_lastMousePos.x - updateData.mousePos.x;
			mouseMovement.y = updateData.mousePos.y - m_lastMousePos.y;
			mouseMovement *= 0.1f; // 遅くする
			// X軸回転
			if (((m_cameraFront.y > -0.99) && (glm::radians(mouseMovement.y) > 0)) || ((m_cameraFront.y < 0.99) && (glm::radians(mouseMovement.y) < 0)))
			{
				m_cameraFront = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(mouseMovement.y), m_cameraRight) * glm::vec4(m_cameraFront, 0.0f));
				m_cameraRight = glm::normalize(glm::cross(m_cameraFront, m_cameraUp));
			}
			// Y軸回転
			m_cameraFront = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(mouseMovement.x), m_cameraUp) * glm::vec4(m_cameraFront, 0.0f));
			m_cameraRight = glm::normalize(glm::cross(m_cameraFront, m_cameraUp));
		}
		m_lastMousePos = updateData.mousePos;

		// 移動
		float speed = AppInfo::g_moveSpeed;
		glm::vec3 move = glm::vec3(0.0f);

		{// boost (Shift)
			auto key = updateData.keyActions.find(GLFW_KEY_LEFT_SHIFT);
			if (key != updateData.keyActions.end())
			{
				if ((key->second == GLFW_PRESS) || (key->second == GLFW_REPEAT))
				{
					speed = AppInfo::g_moveBoostSpeed;
				}
			}
		}

		speed *= updateData.deltaTime;
		{// W
			auto key = updateData.keyActions.find(GLFW_KEY_W);
			if (key != updateData.keyActions.end())
			{
				if ((key->second == GLFW_PRESS) || (key->second == GLFW_REPEAT))
				{
					move.x += speed * m_cameraFront.x;
					move.z += speed * m_cameraFront.z;
				}
			}
		}
		{// A
			auto key = updateData.keyActions.find(GLFW_KEY_A);
			if (key != updateData.keyActions.end())
			{
				if ((key->second == GLFW_PRESS) || (key->second == GLFW_REPEAT))
				{
					move.x -= speed * m_cameraRight.x;
					move.z -= speed * m_cameraRight.z;
				}
			}
		}
		{// S
			auto key = updateData.keyActions.find(GLFW_KEY_S);
			if (key != updateData.keyActions.end())
			{
				if ((key->second == GLFW_PRESS) || (key->second == GLFW_REPEAT))
				{
					move.x -= speed * m_cameraFront.x;
					move.z -= speed * m_cameraFront.z;
				}
			}
		}
		{// D
			auto key = updateData.keyActions.find(GLFW_KEY_D);
			if (key != updateData.keyActions.end())
			{
				if ((key->second == GLFW_PRESS) || (key->second == GLFW_REPEAT))
				{
					move.x += speed * m_cameraRight.x;
					move.z += speed * m_cameraRight.z;
				}
			}
		}
		{// Q
			auto key = updateData.keyActions.find(GLFW_KEY_Q);
			if (key != updateData.keyActions.end())
			{
				if ((key->second == GLFW_PRESS) || (key->second == GLFW_REPEAT))
				{
					move.y -= speed * m_cameraUp.y;
				}
			}
		}
		{// E
			auto key = updateData.keyActions.find(GLFW_KEY_E);
			if (key != updateData.keyActions.end())
			{
				if ((key->second == GLFW_PRESS) || (key->second == GLFW_REPEAT))
				{
					move.y += speed * m_cameraUp.y;
				}
			}
		}
		m_cameraPos += move;

		editorGUI();
	}

	void EditorCamera::initialize()
	{
		__super::initialize();
		m_lastMousePos = glm::vec3(0.0f);
	}

	void EditorCamera::editorGUI()
	{
		//ImGui::Begin("MainEditorWindow");
		//if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
		//{
		//	ImGui::Text("CameraPos: %.2f, %.2f, %.2f", m_cameraPos.x, m_cameraPos.y, m_cameraPos.z);
		//	ImGui::Text("CameraFront: %.2f, %.2f, %.2f", m_cameraFront.x, m_cameraFront.y, m_cameraFront.z);
		//	ImGui::Text("CameraRight: %.2f, %.2f, %.2f", m_cameraRight.x, m_cameraRight.y, m_cameraRight.z);
		//	ImGui::Text("CameraUp: %.2f, %.2f, %.2f", m_cameraUp.x, m_cameraUp.y, m_cameraUp.z);
		//	ImGui::Text("CameraFov: %.2f", m_cameraFov);
		//	ImGui::Text("CameraNear: %.2f", m_cameraNear);
		//	ImGui::Text("CameraFar: %.2f", m_cameraFar);
		//}
		//ImGui::End();
	}
}