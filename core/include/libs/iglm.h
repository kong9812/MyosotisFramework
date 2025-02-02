// Copyright (c) 2025 kong9812
// For include glm
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace glm
{
    // 面の法線を求める(正規化)
	inline glm::vec4 normalizePlane(glm::vec4 plane) 
    {
        float length = glm::length(plane);
        return plane / length;
    }
}