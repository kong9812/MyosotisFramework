#version 450
#extension GL_GOOGLE_include_directive : require

#include "Descriptors/CameraInfo.glsl"

layout (location = 0) in vec2 inNDC;
layout (location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstant {
    mat4 invVP;
};

// 定数定義
const float PLANE_Y = 0.0;
const float SPACING = 1.0;
const float LINE_WIDTH_PX = 0.5;    // 1がいいかも…悩む
const vec3 GRID_COLOR = vec3(1.0);
const float GRID_ALPHA = 0.8;

float gridLineHard(vec2 worldXZ, float spacing, float thicknessWorld)
{
    // fractで0-1のループを作り、0.5(セル中央)からの距離を見る
    vec2 coord = worldXZ / spacing;
    vec2 a = abs(fract(coord - 0.5) - 0.5);
    float d = min(a.x, a.y);

    // ワールド単位の太さをセル単位(0.0-0.5)に変換
    float thicknessCell = clamp(thicknessWorld / spacing, 0.0, 0.5);
    
    // smoothstepでピクセル跨ぎの際のチラつきを抑える
    return smoothstep(thicknessCell, 0.0, d);
}

void main() 
{
    // レイ復元 (Near平面(Z=0)とFar平面(Z=1)を逆投影)
    vec4 near = invVP * vec4(inNDC, 0.0, 1.0);
    vec4 far  = invVP * vec4(inNDC, 1.0, 1.0);

    vec3 rayOrigin = near.xyz / near.w;
    vec3 rayDir = normalize((far.xyz / far.w) - rayOrigin);

    // 真横(水平)は描画不能
    if (abs(rayDir.y) < 1e-6) discard;

    // 地面(Y=PLANE_Y)との交点tを計算
    float t = (PLANE_Y - rayOrigin.y) / rayDir.y;
    if (t <= 0.0) discard;

    // ワールド座標の決定
    vec3 worldPosition = rayOrigin + rayDir * t;
    vec2 worldPositionXZ = worldPosition.xz;

    // 1px相当のワールド (厚み？) 太さを同じするため
    vec2 dx = dFdx(worldPositionXZ);
    vec2 dy = dFdy(worldPositionXZ);
    float worldPrePixel = max(length(dx), length(dy));
    worldPrePixel = max(worldPrePixel, 1e-6);
    float thicknessWorld = worldPrePixel * LINE_WIDTH_PX;

    // グリッド強度
    float line = gridLineHard(worldPositionXZ, SPACING, thicknessWorld);

    // 深度を書き込む
    CameraData cameraData = CameraInfo_GetCameraData(CameraInfo_GetMainCameraIndex());
    vec4 clipPos = cameraData.projection * cameraData.view * vec4(worldPosition, 1.0);
    gl_FragDepth = clipPos.z / clipPos.w;

    // 距離フェード
    float dist = length(worldPosition - rayOrigin);
    float fade = exp(-0.015 * dist);

    outColor = vec4(GRID_COLOR, line * GRID_ALPHA * fade);
    
    if (outColor.a < 0.01) discard;
}