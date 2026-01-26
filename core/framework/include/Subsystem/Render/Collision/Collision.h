// Copyright (c) 2025 kong9812
#pragma once
#include "iglm.h"
#include <algorithm>

namespace MyosotisFW::System::Render
{
	// 線分とレイの最短距離
	inline float DistanceRayToSegment(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& p0, const glm::vec3& p1, float& outU)
	{
		glm::vec3 v = p1 - p0;
		glm::vec3 w = rayOrigin - p0;

		float a = glm::dot(v, v);
		float b = glm::dot(v, rayDir);
		float c = glm::dot(rayDir, rayDir);
		float d = glm::dot(v, w);
		float e = glm::dot(rayDir, w);

		float denom = a * c - b * b;

		float s = 0.0f;
		if (denom < 1e-6f)
		{
			// ほぼ平行な場合
			s = d / a;
		}
		else
		{
			s = (c * d - b * e) / denom;
		}

		// 線分なので、sが0.0～1.0の間に収まるようにする
		outU = std::clamp(s, 0.0f, 1.0f);

		// レイ上の最短点
		float t = (b * outU - e) / c;

		if (t < 0.0f)
		{
			t = 0.0f;
			outU = std::clamp(glm::dot((rayOrigin - p0), v) / a, 0.0f, 1.0f);
		}

		glm::vec3 closestPointOnSegment = p0 + outU * v;
		glm::vec3 closestPointOnRay = rayOrigin + t * rayDir;

		// 二点間の距離を返す
		return glm::distance(closestPointOnSegment, closestPointOnRay);
	}

	// 線分とレイの当たり判定
	// threshold: 線分の太さ
	bool RaySegmentCollision(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& p0, const glm::vec3& p1, const float threshold)
	{
		float outU{};
		float dist = DistanceRayToSegment(rayOrigin, rayDir, p0, p1, outU);

		// 最短距離がしきい値以内なら「当たった」とみなす
		return dist < threshold;
	}
}