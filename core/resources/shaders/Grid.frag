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
const vec3 GRID_COLOR = vec3(0.0, 0.0, 0.0);
const float GRID_ALPHA = 0.8;

// worldXZ: ワールド座標のXZ
// spacing: マスの隙間
// thicknessWorld: 線の太さ
float gridLineHard(vec2 worldXZ, float spacing, float thicknessWorld)
{
    // ワールド座標のXZ -> マス単位に変換 マス(X,Y)
    vec2 coord = worldXZ / spacing;
    // fract(coord) 小数部分を取り出す
    // マスの中心: 0.5
    // マスの線: 0.0
    // マスの線までの距離を計算↓
    vec2 a = abs(fract(coord - 0.5) - 0.5);

    // 線のマス上の割合: thicknessWorld / spacing (範囲: 0.0～0.5)
    float thicknessCell = clamp(thicknessWorld / spacing, 0.0, 0.5);

    // 縦線・横線　近いのを使う
    float d = min(a.x, a.y);
    
    // 線からの距離 d が thicknessCell未満なら線
    // d = 0.0 (線 -> 1)
    // d >= thicknessCell (線から離れる -> 0)
    // 線:1 空白:0
    // 境界は smoothstepでフェード (AAぽく)
    return smoothstep(thicknessCell, 0.0, d);
}

void main() 
{
    // レイ復元
    vec4 near = invVP * vec4(inNDC, 0.0, 1.0);      // 画面用のpxのdepth 0(一番近い) ワールド位置 (同次座標)
    vec4 far  = invVP * vec4(inNDC, 1.0, 1.0);      // 画面用のpxのdepth 1(一番遠い) ワールド位置 (同次座標)
    vec3 rayOrigin = near.xyz / near.w;             // 同次座標 -> ワールド座標
    vec3 rayDir = normalize((far.xyz / far.w) - rayOrigin);     // origin->far の方向

    // 水平チェック (Y方向に進まない)　交差しない
    // ここが0なら↓の除算に問題が起きるから事前に計算
    if (abs(rayDir.y) < 1e-6) discard;

    // 地面(Y=PLANE_Y)との交点tを計算
    // p(t) = rayOrigin + (rayDir * t)
    // p(t).y = rayOrigin.y + (rayDir.y * t)
    // p(t).y = PLANE_Y
    // ↓
    float t = (PLANE_Y - rayOrigin.y) / rayDir.y;
    if (t <= 0.0) discard;  // 水平 -> 描画しない

    // 交点のワールド座標
    vec3 worldPosition = rayOrigin + rayDir * t;
    vec2 worldPositionXZ = worldPosition.xz;

    // 画面上の線をピクセル一定にするため、1px相当のワールド距離から太さを決める
    vec2 dx = dFdx(worldPositionXZ);    // 隣pxのworldPositionXZの差分
    vec2 dy = dFdy(worldPositionXZ);    // 隣pxのworldPositionXZの差分
    float worldPerPixel = max(length(dx), length(dy));  // 大きい方を使う
    worldPerPixel = max(worldPerPixel, 1e-6);   // 0=微分が取れない (0除算防止)
    // ピクセル太さ -> ワールド太さ (描画上の太さを一定に)
    // 遠いほど worldPerPixelが太くなる
    float thicknessWorld = worldPerPixel * LINE_WIDTH_PX;

    // グリッド強度
    float line = gridLineHard(worldPositionXZ, SPACING, thicknessWorld);

    // 深度を書き込む
    CameraData cameraData = CameraInfo_GetCameraData(CameraInfo_GetMainCameraIndex());
    vec4 clipPos = cameraData.projection * cameraData.view * vec4(worldPosition, 1.0);
    gl_FragDepth = clipPos.z / clipPos.w;

    // 距離フェード
    // dist: 交点 <-> nearワールド座標 の距離
    // rayOriginはカメラposに変更にいいが nearを使うと値が近く、誤差が小さい
    float dist = length(worldPosition - rayOrigin);
    // 遠くなると薄く (係数: -0.015 0.015から薄く)
    float fade = exp(-0.15 * dist);

    // a = グリッド強度 * GRID_ALPHA(デフォルト) * 距離フェード
    outColor = vec4(GRID_COLOR, line * GRID_ALPHA * fade);
    
    // ほぼ透明描画しない
    if (outColor.a < 0.01) discard;
}