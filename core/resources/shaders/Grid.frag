#version 450
#extension GL_GOOGLE_include_directive : require

#include "Descriptors/CameraInfo.glsl"

layout (location = 0) in vec3 inNearPoint;
layout (location = 1) in vec3 inFarPoint;

layout (location = 0) out vec4 outColor;

// 定数定義
const float GRID_SCALE = 1.0;          // グリッドの密度
const vec4 GRID_COLOR = vec4(1.0, 1.0, 1.0, 1.0); // グリッドの色（グレー）
const float FADE_DIST = 0.01;         // 遠くの消え方（小さいほど遠くまで見える）

void main() 
{
    // y = 0.0 平面との交点計算
    float t = -inNearPoint.y / (inFarPoint.y - inNearPoint.y);
    if (t <= 0.0 || isinf(t)) discard;

    vec3 fragPos3D = inNearPoint + t * (inFarPoint - inNearPoint);

    // 深度の計算と書き込み
    CameraData cameraData = CameraInfo_GetCameraData(CameraInfo_GetMainCameraIndex());
    vec4 clip_space_pos = cameraData.projection * cameraData.view * vec4(fragPos3D, 1.0);
    gl_FragDepth = clip_space_pos.z / clip_space_pos.w;

    // アンチエイリアスが効いたグリッド線の計算
    vec2 coord = fragPos3D.xz * GRID_SCALE;
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    float alpha = 1.0 - min(line, 1.0);

    // 距離によるフェードアウト
    float fade = exp(-FADE_DIST * length(fragPos3D.xz));

    // 出力色の計算
    outColor = GRID_COLOR;
    outColor.a *= (alpha * fade);

    // 軸（X=0, Z=0）を少し強調する場合
    if (abs(fragPos3D.x) < 0.05 || abs(fragPos3D.z) < 0.05) {
        outColor.a += 0.2 * fade;
    }

    if (outColor.a < 0.01) discard;
}