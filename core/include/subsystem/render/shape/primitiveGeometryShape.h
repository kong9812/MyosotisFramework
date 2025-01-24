// Copyright (c) 2025 kong9812
#pragma once
#include <vector>

#include "iglm.h"
#include "vkStruct.h"

namespace MyosotisFW::System::Render::Shape
{
    inline Utility::Vulkan::Struct::Vertex createQuad(float size = 1.0f, glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f }, glm::vec3 center = { 0.0f, 0.0f, 0.0f })
    {
        float halfSize = size * 0.5f;

        Utility::Vulkan::Struct::Vertex vertexAndIndex = {};

        vertexAndIndex.vertex = {
            -halfSize + center.x, -halfSize + center.y, -halfSize + center.z, 1.0  , color.r, color.g, color.b, color.a,    // 0
            halfSize + center.x, -halfSize + center.y, -halfSize + center.z, 1.0, color.r, color.g, color.b, color.a,       // 1
            halfSize + center.x, halfSize + center.y, -halfSize + center.z, 1.0, color.r, color.g, color.b, color.a,        // 2
            -halfSize + center.x, halfSize + center.y, -halfSize + center.z, 1.0, color.r, color.g, color.b, color.a,       // 3

            -halfSize + center.x, -halfSize + center.y, halfSize + center.z, 1.0, color.r, color.g, color.b, color.a,       // 4
            halfSize + center.x, -halfSize + center.y, halfSize + center.z, 1.0, color.r, color.g, color.b, color.a,        // 5
            halfSize + center.x, halfSize + center.y, halfSize + center.z, 1.0, color.r, color.g, color.b, color.a,         // 6
            -halfSize + center.x, halfSize + center.y, halfSize + center.z, 1.0, color.r, color.g, color.b, color.a,        // 7
        };

        vertexAndIndex.index = {
            0, 2, 1, 0, 3, 2,     // 前面
            4, 5, 6, 4, 6, 7,     // 背面
            0, 1, 5, 0, 5, 4,     // 左面
            2, 3, 7, 2, 7, 6,     // 右面
            3, 0, 4, 3, 4, 7,     // 上面
            1, 2, 6, 1, 6, 5      // 底面
        };
        return vertexAndIndex;
    }
}


