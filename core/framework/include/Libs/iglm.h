// Copyright (c) 2025 kong9812
// For include glm
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace glm
{
	// 面の法線を求める(正規化)
	inline glm::vec4 normalizePlane(const glm::vec4& plane)
	{
		float length = glm::length(glm::vec3(plane));
		return{ glm::vec3(plane) / length, plane.w / length };
	}
}