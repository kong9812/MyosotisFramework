// Copyright (c) 2025 kong9812
#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "vkStruct.h"

namespace MyosotisFW::System::Render::Shape
{
    inline Utility::Vulkan::Struct::VertexAndIndex createQuad(float size, glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f }, glm::vec3 center = { 0.0f, 0.0f, 0.0f })
    {
        float halfSize = size * 0.5f;

        Utility::Vulkan::Struct::VertexAndIndex vertexAndIndex = {};

        vertexAndIndex.vertex = {
            {glm::vec4(-halfSize + center.x, -halfSize + center.y, -halfSize + center.z, 1.0)  , color},      // 0
            {glm::vec4(halfSize + center.x, -halfSize + center.y, -halfSize + center.z, 1.0), color},      // 1
            {glm::vec4(halfSize + center.x, halfSize + center.y, -halfSize + center.z, 1.0), color},      // 2
            {glm::vec4(-halfSize + center.x, halfSize + center.y, -halfSize + center.z, 1.0), color},      // 3
            {glm::vec4(-halfSize + center.x, -halfSize + center.y, halfSize + center.z, 1.0), color},      // 4
            {glm::vec4(halfSize + center.x, -halfSize + center.y, halfSize + center.z, 1.0), color},      // 5
            {glm::vec4(halfSize + center.x, halfSize + center.y, halfSize + center.z, 1.0), color},      // 6
            {glm::vec4(-halfSize + center.x, halfSize + center.y, halfSize + center.z, 1.0), color},      // 7
        };

        vertexAndIndex.index = {
            glm::vec3(0, 2, 1), glm::vec3(0, 3, 2),     // 前面
            glm::vec3(4, 5, 6), glm::vec3(4, 6, 7),     // 背面
            glm::vec3(0, 1, 5), glm::vec3(0, 5, 4),     // 左面
            glm::vec3(2, 3, 7), glm::vec3(2, 7, 6),     // 右面
            glm::vec3(3, 0, 4), glm::vec3(3, 4, 7),     // 上面
            glm::vec3(1, 2, 6), glm::vec3(1, 6, 5)      // 底面
        };
        return vertexAndIndex;
    }
}


