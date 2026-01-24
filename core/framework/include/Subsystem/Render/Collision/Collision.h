// Copyright (c) 2025 kong9812
#pragma once
#include "iglm.h"
#include <algorithm>

namespace MyosotisFW::System::Render
{
	// 線分とレイの最短距離
	inline float DistanceRayToSegment(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& p0, const glm::vec3& p1, float& outU)
	{
		// 線分のベクトル
		glm::vec3 v = p1 - p0;
		// 線分の始点からレイの始点へのベクトル
		glm::vec3 w = rayOrigin - p0;

		// 内積による幾何学的情報の抽出
		// a: 線分の長さの2乗 (|v|^2)
		// b: 線分ベクトルとレイ方向の内積 (v・dir)
		// c: レイ方向の長さの2乗 (|dir|^2 = 1.0)
		// d: 線分ベクトルとwの内積 (v・w)
		// e: レイ方向とwの内積 (dir・w)
		float a = glm::dot(v, v);				// 軸の長さ
		float b = glm::dot(v, rayDir);			// 軸と視線の角度		視線内にある場合の前後関係を探す +: 前 0: 同じ方向 -: 後ろ
		float c = glm::dot(rayDir, rayDir);		// 視線の長さ			正規化済みのため、1だね
		float d = glm::dot(v, w);				// 軸から見た前後		軸vへのwの投影。p0から垂線の足までの位置(vの長さ倍)
		float e = glm::dot(rayDir, w);			// 視線から見た前後		+: ギズモが視線の先にある -: ギズモが視線の後ろにある

		// 例: ギズモの軸 vの長さを10、正規化された視線rayDirの長さが1とする
		// a: dot(v,v) = 10*10 = 100
		// b(平行): |v|*|rayDir|*cosΘ Θ:0(平行)、b = 10*1*1 = 10
		// b = v
		// b^2 = v^2 = a * c
		// denom = 0
		// 平行！！
		float denom = a * c - b * b;

		float s = 0.0f;
		if (denom < 1e-6f)
		{
			// ほぼ平行な場合
			s = 0.0f;
		}
		else
		{
			// (b * e - c * d):　(軸と視線の角度 * 視線から見た前後) - (視線の長さ * 軸から見た前後)
			// denom: 2つの線の広がり (面積)
			s = (b * e - c * d) / denom;
		}

		// 線分なので、sが0.0～1.0の間に収まるようにする
		outU = std::clamp(s, 0.0f, 1.0f);

		// 線分上の最短点
		glm::vec3 closestPointOnSegment = p0 + outU * v;

		// レイ上の最短点
		float t = (s * b - e) / c;
		// レイは始点から前方(t > 0)にしか伸びないので max(0.0, t) で制限する
		glm::vec3 closestPointOnRay = rayOrigin + std::max(0.0f, t) * rayDir;

		// 二点間の距離を返す
		return glm::distance(closestPointOnSegment, closestPointOnRay);
	}

	// 線分とレイの当たり判定
	// threshold: 線分の太さ
	bool RaySegmentCollision(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& p0, const glm::vec3& p1, const float threshold = 0.1f)
	{
		float outU{};
		float dist = DistanceRayToSegment(rayOrigin, rayDir, p0, p1, outU);

		// 最短距離がしきい値以内なら「当たった」とみなす
		return dist < threshold;
	}
}