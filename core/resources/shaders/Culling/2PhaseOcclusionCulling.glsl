#ifndef TWOPHASEOCCLUSIONCULLING
#define TWOPHASEOCCLUSIONCULLING
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_debug_printf : enable

#include "../Loader/Sampler2DLoader.glsl"
#include "../Loader/MainCameraDataLoader.glsl"

// 共有バッファ
// memo: 計算はそれぞれやってから共有バッファに値を入れる
shared vec2 sMinUV;
shared vec2 sMaxUV;
shared float sMinDepth;
shared uint sPrevDepthMax_u;
shared float sPrevDepthMax;
shared bool sVisible;

// 8頂点の一時置き場（共有配列）
shared vec2  sLocalUV[8];
shared float sLocalDepth[8];

// マルチスレッド
bool TwoPhaseOcclusionCulling_IsVisible(vec3 aabbMin, vec3 aabbMax, StandardSSBO standardSSBO, uint hiZSamplerID, int mipLevel)
{
    // forループの際にスレッドそれぞれのインクリメント値
    const uint stride = gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z;
    
    // カメラデータ取得
    MainCameraData cameraData = MainCameraDataLoader_GetMainCameraData();

    // 初期化 (0番目のみ実行する)
    if (gl_LocalInvocationIndex == 0)
    {
        sMinUV = vec2( 1.0);    // [スクリーンスペース] 矩形の範囲 min
        sMaxUV = vec2(-1.0);    // [スクリーンスペース] 矩形の範囲 max
        sMinDepth = 1.0;        // 矩形の最小深度
        sPrevDepthMax_u = 0;    // depthBufferの最大深度
        sVisible = false;
    }
    barrier();

    // AABBの8頂点をスクリーン座標に投影し、矩形を作る (最初の8スレッドのみ計算)
    // ローカルバッファ
    if (gl_LocalInvocationIndex < 8)
    {    
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
        vec4 clip = cameraData.projection * cameraData.view * (standardSSBO.model * vec4(corners[gl_LocalInvocationIndex], 1.0));
        vec3 ndc = clip.xyz / clip.w;   // クリップ座標 -> NDC [-1~1]
        vec2 uv = ndc.xy * 0.5 + 0.5;   // NDC座標 -> UV [0~1]
        sLocalUV[gl_LocalInvocationIndex] = uv;        
        sLocalDepth[gl_LocalInvocationIndex] = ndc.z;
        // debugPrintfEXT("sLocalDepth[%d]: %f", gl_LocalInvocationIndex, sLocalDepth[gl_LocalInvocationIndex]);
    }
    barrier();

    // atomicMinとatomicMaxを使わずthread==0で処理してもらう
    if (gl_LocalInvocationIndex == 0)
    {
        sMinUV = sLocalUV[0];
        sMaxUV = sLocalUV[0];
        sMinDepth = sLocalDepth[0];
        for (uint i = 1; i < 8; i++)
        {
            sMinUV.x = min(sMinUV.x, sLocalUV[i].x);
            sMinUV.y = min(sMinUV.y, sLocalUV[i].y);
            sMaxUV.x = max(sMaxUV.x, sLocalUV[i].x);
            sMaxUV.y = max(sMaxUV.y, sLocalUV[i].y);
            sMinDepth = min(sMinDepth, sLocalDepth[i]);
        }
        sMinUV = clamp(sMinUV, vec2(0.0), vec2(1.0));
        sMaxUV = clamp(sMaxUV, vec2(0.0), vec2(1.0));
    }
    barrier();

    // ここからは一番重く、スレッド分割が期待できるところ
    // デップステクスチャのサイズ (mip指定)
    ivec2 mipTexSize = Sampler2DLoader_GetTextureSize(hiZSamplerID, mipLevel);

    // depth画像のテクセル位置に変換
    ivec2 minTexel = ivec2(sMinUV * vec2(mipTexSize));
    ivec2 maxTexel = ivec2(sMaxUV * vec2(mipTexSize));

    // 制限
    minTexel = max(minTexel, ivec2(0));
    maxTexel = min(maxTexel, mipTexSize - ivec2(1));
    barrier();

    // AABB矩形が0なら見える
    if (maxTexel.x <= minTexel.x || maxTexel.y <= minTexel.y) 
    {
        if (gl_LocalInvocationIndex == 0) {
            sVisible = true;
        }
        barrier();
        return sVisible;
    }
    barrier();

    // テクセル数
    uint width  = uint(maxTexel.x - minTexel.x + 1);
    uint height = uint(maxTexel.y - minTexel.y + 1);
    uint totalSize = width * height;

    // ローカルスレッドの最大
    float localMax = -1.0;

    // いよいよfor
    for (uint i = gl_LocalInvocationIndex; i < totalSize; i += stride)
    {
        uint ix = i % width;  // x (eg: 10*20の画像なら 10~19の時 0~9が出てくる、x軸の位置)
        uint iy = i / width;  // y (eg: 10*20の画像なら 12の時 1出てくる、y軸の位置)
        ivec2 tc = ivec2(int(ix) + minTexel.x, int(iy) + minTexel.y);   // depthにあるAABBの位置の左上位置から計算
        float d = texelFetch(Sampler2D[hiZSamplerID], tc, mipLevel).r;
        localMax = max(localMax, d);
    }

    uint localU = (localMax < 0.0) ? 0 : floatBitsToUint(localMax);
    atomicMax(sPrevDepthMax_u, localU);
    barrier();

    // 判定
    if (gl_LocalInvocationIndex == 0)
    {
        sPrevDepthMax = uintBitsToFloat(sPrevDepthMax_u);
        sVisible = (sMinDepth <= sPrevDepthMax);        
    }
    barrier();

    return sVisible;
}
#endif