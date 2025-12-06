#version 450
#extension GL_GOOGLE_include_directive : require

#include "Descriptors/CameraInfo.glsl"

layout (location = 0) in vec4 inWorldPos;
layout (location = 1) in vec3 inWorldNormal;

layout (location = 0) out vec4 outColor;

void main() 
{
    // CameraInfo (MainCameraData) を取得
    CameraData cameraData = CameraInfo_GetCameraData(CameraInfo_GetMainCameraIndex());

    vec3 color = vec3(0.0);

    // 指向ライト
    vec3 lightDir = normalize(vec3(0.3, -1.0, 0.2));  // 光の向き
    vec3 L = normalize(-lightDir);  // 光が当たる方向
    vec3 lightColor = vec3(1.0);

    // DiffuseColor
    float NdotL = max(dot(inWorldNormal, L), 0.0);
    vec3 diffuse = color.rgb * lightColor * NdotL;

    // SpecularColor (Blinn-Phong)
    vec3 V = normalize(cameraData.pos.xyz - inWorldPos.xyz);
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(inWorldNormal, H), 0.0), 32.0);
    vec3 specular = spec * lightColor;

    outColor = vec4(diffuse + specular, 1.0);;
}