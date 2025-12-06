#version 450
#extension GL_GOOGLE_include_directive : require

#include "Loader/Usampler2DLoader.glsl"

#include "Descriptors/VBDispatchInfo.glsl"
#include "Descriptors/ObjectInfo.glsl"
#include "Descriptors/MeshInfo.glsl"
#include "Descriptors/MeshletInfo.glsl"
#include "Descriptors/CameraInfo.glsl"
#include "Descriptors/ScreenInfo.glsl"

#include "Descriptors/VertexData.glsl"
#include "Descriptors/UniqueIndexData.glsl"
#include "Descriptors/PrimitivesData.glsl"

layout (push_constant) uniform PushConstant {
    uint visibilityBufferID;
};

layout (location = 0) out vec4 outColor;

const uint PRIM_BITS = 7u;                      // 2^7 = 128 > 124
const uint PRIM_MASK = (1u << PRIM_BITS) - 1u;  // 0x7F = 127

// 重心座標
vec3 GetBarycentrics(vec2 P, vec2 A, vec2 B, vec2 C)
{
    vec2 v0 = B - A;
    vec2 v1 = C - A;
    vec2 v2 = P - A;

    float d00 = dot(v0, v0);   // |AB|^2
    float d01 = dot(v0, v1);   // AB · AC
    float d11 = dot(v1, v1);   // |AC|^2
    float d20 = dot(v2, v0);   // AP · AB
    float d21 = dot(v2, v1);   // AP · AC

    float denom = d00 * d11 - d01 * d01;

    float w1 = (d11 * d20 - d01 * d21) / denom;
    float w2 = (d00 * d21 - d01 * d20) / denom;
    float w0 = 1.0 - w1 - w2;

    return vec3(w0, w1, w2);
}

// 補完済みピクセル頂点情報
VertexData InterpolateVertexAttributes(VertexData v0, VertexData v1, VertexData v2, vec3 bary) 
{
    VertexData vertexData;
    vertexData.position =
          v0.position.xyzw * bary.x +
          v1.position.xyzw * bary.y +
          v2.position.xyzw * bary.z +
          vec4(0.0);
    vertexData.normal = normalize(
          v0.normal * bary.x +
          v1.normal * bary.y +
          v2.normal * bary.z);
    vertexData.uv =
          v0.uv * bary.x +
          v1.uv * bary.y +
          v2.uv * bary.z;
    vertexData.color =
          v0.color * bary.x +
          v1.color * bary.y +
          v2.color * bary.z;
    return vertexData;
}

vec4 CalcLighting(VertexData vertex, ObjectInfo objectInfo, CameraData cameraData)
{
    // ワールド座標
    vec3 worldPos = (objectInfo.model * vec4(vertex.position.xyz, 1.0)).xyz;

    // ワールドノーマル
    mat3 normalMatrix = mat3(transpose(inverse(objectInfo.model)));
    vec3 worldNormal = normalize(normalMatrix * vertex.normal);

    // 指向ライト
    vec3 lightDir = normalize(vec3(0.3, -1.0, 0.2));  // 光の向き
    vec3 L = normalize(-lightDir);  // 光が当たる方向
    vec3 lightColor = vec3(1.0);

    // DiffuseColor
    float NdotL = max(dot(worldNormal, L), 0.0);
    vec3 diffuse = vertex.color.rgb * lightColor * NdotL;

    // SpecularColor (Blinn-Phong)
    vec3 V = normalize(cameraData.pos.xyz - worldPos);
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(worldNormal, H), 0.0), 32.0);
    vec3 specular = spec * lightColor;

    return vec4(diffuse + specular, 1.0);
}

void main() 
{
    // VB
    ivec2 screenPos = ivec2(gl_FragCoord.xy);
    uint vb = Usampler2DLoader_TexelFetch(visibilityBufferID, screenPos, 0).r;
    
    // 無効判定
    if(vb == 0)  discard;

    // VisibilityBuffer.fragで無効判定のために足した1を引く
    vb -= 1;

    uint primIndex = vb & PRIM_MASK;
    uint vbDispatchInfoIndex = vb >> PRIM_BITS;

    // VBDispatchInfo
    VBDispatchInfo dispactchInfo = VBDispatchInfo_GetVBDispatchInfo(vbDispatchInfoIndex);

    // ObjectInfo
    uint objectID = dispactchInfo.objectID;
    ObjectInfo objectInfo = ObjectInfo_GetObjectInfo(dispactchInfo.objectID);

    // MeshInfo
    uint meshID = dispactchInfo.meshID;
    MeshInfo meshInfo = MeshInfo_GetMeshInfo(meshID);

    // MeshletInfo
    uint meshletID = meshInfo.meshletInfoOffset + dispactchInfo.meshletID;
    MeshletInfo meshletInfo = MeshletInfo_GetMeshletInfo(meshletID);

    // CameraInfo (MainCameraData)
    CameraData cameraData = CameraInfo_GetCameraData(CameraInfo_GetMainCameraIndex());
    mat4 projView = cameraData.projection * cameraData.view;

    // ScreenInfo
    ScreenInfo screenInfo = ScreenInfo_GetScreenInfo(0);

    // プリミティブインデックス
    uvec3 index = PrimitivesData_GetPrimitivesData(meshletInfo.primitivesOffset, primIndex);

    // プリミティブの3頂点
    VertexData vertex[3];
    uint uniqueIndexIndex = UniqueIndexData_GetUniqueIndexData(meshletInfo.uniqueIndexOffset, index.x);
    vertex[0] = VertexData_GetVertexData(meshletInfo.vertexDataOffset, meshInfo.vertexAttributeBit, meshInfo.unitSize, uniqueIndexIndex);
    uniqueIndexIndex = UniqueIndexData_GetUniqueIndexData(meshletInfo.uniqueIndexOffset, index.y);
    vertex[1] = VertexData_GetVertexData(meshletInfo.vertexDataOffset, meshInfo.vertexAttributeBit, meshInfo.unitSize, uniqueIndexIndex);
    uniqueIndexIndex = UniqueIndexData_GetUniqueIndexData(meshletInfo.uniqueIndexOffset, index.z);
    vertex[2] = VertexData_GetVertexData(meshletInfo.vertexDataOffset, meshInfo.vertexAttributeBit, meshInfo.unitSize, uniqueIndexIndex);

    // 重心座標
    vec2 P = gl_FragCoord.xy + vec2(0.5);   // ピクセル中心
    P /= vec2(screenInfo.screenSize);

    // スクリーン座標
    vec2 vertexScreenUV[3];
    vec4 clip = projView * objectInfo.model * vertex[0].position;
    vec3 ndc = clip.xyz / clip.w;
    vertexScreenUV[0] = ndc.xy * 0.5 + 0.5;
    clip = projView * objectInfo.model * vertex[1].position;
    ndc = clip.xyz / clip.w;
    vertexScreenUV[1] = ndc.xy * 0.5 + 0.5;
    clip = projView * objectInfo.model * vertex[2].position;
    ndc = clip.xyz / clip.w;
    vertexScreenUV[2] = ndc.xy * 0.5 + 0.5;
    vec3 bary = GetBarycentrics(P, vertexScreenUV[0], vertexScreenUV[1], vertexScreenUV[2]);

    // 補完済み頂点情報
    VertexData interpolateVertex = InterpolateVertexAttributes(vertex[0], vertex[1], vertex[2], bary);

    // ライティング計算
    outColor = CalcLighting(interpolateVertex, objectInfo, cameraData);
}