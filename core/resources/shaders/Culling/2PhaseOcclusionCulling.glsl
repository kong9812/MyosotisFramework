#ifndef TWOPHASEOCCLUSIONCULLING
#define TWOPHASEOCCLUSIONCULLING

#include "../Loader/Sampler2DLoader.glsl"

// マルチスレッド
bool TwoPhaseOcclusionCulling_IsVisible(vec3 aabbMin, vec3 aabbMax, mat4 objectModel, mat4 cameraProjection, mat4 cameraView, uint hiZSamplerID, float hiZMipLevelMax)
{
    //=============================================================================================================================================================================
    // 2. 2PhaseOccCulling    
    //=============================================================================================================================================================================
    vec2 aabbUV[8];
    float aabbDepth[8];
    vec3 corners[8] = vec3[](
        vec3(aabbMin.x, aabbMin.y, aabbMin.z),   // - - -
        vec3(aabbMax.x, aabbMax.y, aabbMax.z),   // + + +
        vec3(aabbMax.x, aabbMin.y, aabbMin.z),   // + - -
        vec3(aabbMin.x, aabbMax.y, aabbMin.z),   // - + -
        vec3(aabbMin.x, aabbMin.y, aabbMax.z),   // - - +
        vec3(aabbMin.x, aabbMax.y, aabbMax.z),   // - + +
        vec3(aabbMax.x, aabbMin.y, aabbMax.z),   // + - +
        vec3(aabbMax.x, aabbMax.y, aabbMin.z)    // + + -
    );  
    for (uint i = 0; i < 8; i++)
    {
        vec4 clip = cameraProjection * cameraView * (objectModel * vec4(corners[i], 1.0));
        vec3 ndc = clip.xyz / clip.w;   // クリップ座標 -> NDC [-1~1]
        vec2 uv = ndc.xy * 0.5 + 0.5;   // NDC座標 -> UV [0~1]
        aabbUV[i] = uv;        
        aabbDepth[i] = ndc.z;
    }   
    vec2 minUV = aabbUV[0];
    vec2 maxUV = aabbUV[0];
    float minDepth = aabbDepth[0];
    for (uint i = 1; i < 8; i++)
    {
        minUV.x = min(minUV.x, aabbUV[i].x);
        minUV.y = min(minUV.y, aabbUV[i].y);
        maxUV.x = max(maxUV.x, aabbUV[i].x);
        maxUV.y = max(maxUV.y, aabbUV[i].y);
        minDepth = min(minDepth, aabbDepth[i]);
    }
    minUV = clamp(minUV, vec2(0.0), vec2(1.0));
    maxUV = clamp(maxUV, vec2(0.0), vec2(1.0)); 
    float localMax = -1.0;  
    float bias = 0.0005;
    ivec2 mip0TextureSize = Sampler2DLoader_GetTextureSize(hiZSamplerID, 0);    
    // AABB Size
    float aabbWidth = (maxUV.x - minUV.x) * float(mip0TextureSize.x);
    float aabbHeight = (maxUV.y - minUV.y) * float(mip0TextureSize.y);
    float maxDim = max(aabbWidth, aabbHeight);  
    float mipLevel = 0.0;
    if (maxDim > 1.0)
    {
        mipLevel = floor(log2(maxDim));
    }
    mipLevel -= 1;
    mipLevel = clamp(mipLevel, 0.0, hiZMipLevelMax);
    ivec2 targetMipTextureSize = Sampler2DLoader_GetTextureSize(hiZSamplerID, int(mipLevel));   
    ivec2 minTexel = ivec2(minUV * targetMipTextureSize) - ivec2(1);    // bias: - ivec2(1) 本当にこれでいいのかなぁ…
    ivec2 maxTexel = ivec2(maxUV * targetMipTextureSize) + ivec2(1);    // bias: + ivec2(1) 本当にこれでいいのかなぁ…
    minTexel = clamp(minTexel, ivec2(0), targetMipTextureSize - ivec2(1));
    maxTexel = clamp(maxTexel, ivec2(0), targetMipTextureSize - ivec2(1));
    bool occCullingVisible = false;
    for (int x = minTexel.x; x <= maxTexel.x; x++)
    {
        for (int y = minTexel.y; y <= maxTexel.y; y++)
        {
            float occluderDepth = Sampler2DLoader_TexelFetch(hiZSamplerID, ivec2(x, y), int(mipLevel)).r;
            if ((occluderDepth + bias) >= minDepth)
            {
                return true;
            }
        }
        if (occCullingVisible) break;
    }
    return false;
}
#endif