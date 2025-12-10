// Copyright (c) 2025 kong9812
#pragma once
#include <ofbx.h>
#include "iglm.h"

inline glm::vec2 ToGlmVec2(const ofbx::Vec2& v)
{
	return glm::vec2(v.x, v.y);
}

inline glm::vec3 ToGlmVec3(const ofbx::Vec3& v)
{
	return glm::vec3(v.x, v.y, v.z);
}

inline glm::vec4 ToGlmVec4(const ofbx::Vec4& v)
{
	return glm::vec4(v.x, v.y, v.z, v.w);
}
