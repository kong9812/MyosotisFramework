// Copyright (c) 2025 kong9812
#pragma once
#include <unordered_map>
#include "iglm.h"

namespace MyosotisFW
{
	struct UpdateData
	{
		bool pause = false;
		float deltaTime = 0.0f;
		std::unordered_map<int, int> keyActions{};
		std::unordered_map<int, int> mouseButtonActions{};
		glm::vec2 mousePos = glm::vec2(0.0f);
		glm::vec2 screenSize = glm::vec2(0.0f);
	};
}