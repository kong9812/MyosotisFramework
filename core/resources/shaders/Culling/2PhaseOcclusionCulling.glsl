#ifndef TWOPHASEOCCLUSIONCULLING
#define TWOPHASEOCCLUSIONCULLING
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_debug_printf : enable

#include "../Loader/Sampler2DLoader.glsl"
#include "../Loader/MainCameraDataLoader.glsl"

//ref: https://sites.google.com/site/monshonosuana/directx%E3%81%AE%E8%A9%B1/directx%E3%81%AE%E8%A9%B1-%E7%AC%AC177%E5%9B%9E?authuser=0
bool TwoPhaseOcclusionCulling_IsVisible(vec3 aabbMin, vec3 aabbMax, StandardSSBO standardSSBO, uint hiZSamplerID, int mipLevel)
{
    MainCameraData cameraData = MainCameraDataLoader_GetMainCameraData();
    // AABBの8頂点をスクリーン座標に投影し、矩形を作る
    vec3 corners[8] = vec3[](
        vec3(aabbMin.x, aabbMin.y, aabbMin.z),   // - - -
        vec3(aabbMax.x, aabbMax.y, aabbMax.z),   // + + +
        vec3(aabbMax.x, aabbMin.y, aabbMin.z),   // + - -
        vec3(aabbMin.x, aabbMax.y, aabbMin.z),   // - + -
        vec3(aabbMin.x, aabbMin.y, aabbMax.z),   // - - +
        vec3(aabbMax.x, aabbMax.y, aabbMin.z),   // + + -
        vec3(aabbMin.x, aabbMax.y, aabbMax.z),   // - + +
        vec3(aabbMax.x, aabbMin.y, aabbMax.z)    // + - +
    );
    // Mesh矩形AABBの最小Depth・AABBのUVを取り出す
    vec2 minUV = vec2( 1.0);    // [スクリーンスペース] 矩形の範囲 min
    vec2 maxUV = vec2(-1.0);    // [スクリーンスペース] 矩形の範囲 max
    float minDepth = 1.0;       // 矩形の最大深度
    for (int i = 0; i < 8; i++) 
    {
        vec4 clip = cameraData.projection * cameraData.view * (standardSSBO.model * vec4(corners[i], 1.0));
        vec3 ndc = clip.xyz / clip.w;   // クリップ座標 -> NDC [-1~1]
        vec2 uv = ndc.xy * 0.5 + 0.5;   // NDC座標 -> UV [0~1]
        minUV = min(minUV, uv);
        maxUV = max(maxUV, uv);
        minDepth = min(minDepth, ndc.z);    // NDC.z: 深度 -> [0~1]に変換
    }

    // UV範囲制限
    minUV = clamp(minUV, vec2(0.0), vec2(1.0));
    maxUV = clamp(maxUV, vec2(0.0), vec2(1.0));

    ivec2 mipTexSize = Sampler2DLoader_GetTextureSize(hiZSamplerID, mipLevel);

    // minUV～maxUVをテクセル座標に変換
    ivec2 minTexel = ivec2(minUV * vec2(mipTexSize));
    ivec2 maxTexel = ivec2(maxUV * vec2(mipTexSize));

    // 範囲内全テクセルでループ
    float prevDepthMax = 0.0;
    for (int y = minTexel.y; y <= maxTexel.y; y++) {
        for (int x = minTexel.x; x <= maxTexel.x; x++) {
            // UVに戻す
            vec2 uv = (vec2(x, y) + 0.5) / vec2(mipTexSize); // ピクセル中心をUVに

            // 深度取得
            float d = texelFetch(Sampler2D[hiZSamplerID], ivec2(x, y), mipLevel).r;
            prevDepthMax = max(prevDepthMax, d);
        }
    }

    return minDepth <= prevDepthMax;
}
#endif