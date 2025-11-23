// Copyright (c) 2025 kong9812
#include "FpsCamera.h"
#include "iglfw.h"
#include "AppInfo.h"

namespace MyosotisFW::System::Render::Camera
{
	FPSCamera::FPSCamera(const uint32_t objectID) : CameraBase(objectID),
		m_lastMousePos(glm::vec2(0.0f))
	{
		m_lastMousePos = glm::vec3(0.0f);
		m_name = "FPSCamera";
	}

	glm::mat4 FPSCamera::GetViewMatrix() const
	{
		return glm::lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);
	}

	glm::mat4 FPSCamera::GetProjectionMatrix() const
	{
		return glm::perspective(glm::radians(m_cameraFov), m_aspectRadio, m_cameraNear, m_cameraFar);
	}

	glm::vec3 FPSCamera::GetWorldPos(const glm::vec2& pos, const float distance) const
	{
		return __super::GetWorldPos(pos, distance);
	}

	void FPSCamera::Update(const UpdateData& updateData)
	{
		if (updateData.pause) return;

		if (!m_isReady)
		{
			// 最初の位置をリセット
			m_lastMousePos = updateData.mousePos;
			m_isReady = true;
		}

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
	}

	void FPSCamera::initialize()
	{
		__super::initialize();
		m_lastMousePos = glm::vec3(0.0f);
	}

	//void FPSCamera::BindDebugGUIElement()
	//{
	//	__super::BindDebugGUIElement();
	//}
}