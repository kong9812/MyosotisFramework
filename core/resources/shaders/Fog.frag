#version 450
#extension GL_GOOGLE_include_directive : require

#include "Descriptors/CameraInfo.glsl"
#include "Descriptors/ScreenInfo.glsl"
#include "Loader/Sampler2DLoader.glsl"

layout (location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstant {
    uint depthTextureID;
    float startDist;
    float endDist;
    float _cp1;
    vec4 fogColor;
};

void main() 
{
    // CameraInfo (MainCameraData)
    CameraData cameraData = CameraInfo_GetCameraData(CameraInfo_GetMainCameraIndex());
    // ScreenInfo
    ScreenInfo screenInfo = ScreenInfo_GetScreenInfo(0);
    // depth
    vec2 UV = gl_FragCoord.xy / screenInfo.screenSize;
    float depth = Sampler2DLoader_GetTexture(depthTextureID, UV).r;

    vec2 ndcXY = vec2(UV.x * 2.0 - 1.0, UV.y * 2.0 - 1.0);
    vec4 crip = vec4(ndcXY, depth, 1.0);
    vec4 view = cameraData.invProjection * crip;
    view /= view.w;

    float dist = length(view.xyz);
    
    // 密度: 0.01
    float fogFactor = 1.0 - exp(-dist * 0.01);

    outColor = vec4(fogColor.xyz, fogFactor);
}