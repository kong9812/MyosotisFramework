// Copyright (c) 2025 kong9812
// For include glm
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace glm
{
	inline glm::vec4 normalizePlane(glm::vec4 plane) {
        float length = glm::length(glm::vec3(plane));
        return plane / length;
    }
}