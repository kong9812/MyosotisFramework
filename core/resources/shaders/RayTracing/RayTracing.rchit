#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require

#include "../Loader/Sampler2DLoader.glsl"

#include "../Descriptors/TLASInstanceInfo.glsl"
#include "../Descriptors/ObjectInfo.glsl"
#include "../Descriptors/MeshInfo.glsl"
#include "../Descriptors/CameraInfo.glsl"

#include "../Descriptors/VertexData.glsl"
#include "../Descriptors/IndexData.glsl"
#include "../Descriptors/BasicMaterialInfo.glsl"

struct RayPayload {
	vec3 color;
	float distance;
};

layout(location = 0) rayPayloadInEXT RayPayload rayPayload;
hitAttributeEXT vec2 attribs;

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
    vertexData.uv0 =
          v0.uv0 * bary.x +
          v1.uv0 * bary.y +
          v2.uv0 * bary.z;
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
    vec3 lightColor = vec3(1.0, 0.85, 0.7);

    // Direct Diffuse
    float NdotL = max(dot(worldNormal, L), 0.0);
    vec3 directDiffuse = lightColor * NdotL;

    // Indriect Diffuse (なんちゃって)
    // todo. レイトレーシングでやるよ
    vec3 skyColor = vec3(1.0, 0.85, 0.7);
    vec3 groundColor = vec3(0.0, 0.0, 0.0);
    float up = clamp(worldNormal.y * 0.5 + 0.5, 0.0, 1.0);
    vec3 indirectDiffuse = mix(skyColor, groundColor, up);

    // DiffuseColor
    vec3 diffuse = vertex.color.rgb * (directDiffuse + indirectDiffuse);

    // SpecularColor (Blinn-Phong)
    vec3 V = normalize(cameraData.pos.xyz - worldPos);
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(worldNormal, H), 0.0), 32.0);
    vec3 specular = spec * lightColor;

    return vec4(diffuse + specular, 1.0);
}

void main()
{
	const vec3 bary = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);

	// PrimID
	uint tri = gl_PrimitiveID;

    // TLASInstanceInfo
    TLASInstanceInfo tlasInstanceInfo = TLASInstanceInfo_GetTLASInstanceInfo(gl_InstanceCustomIndexEXT);

	// ObjectInfo
	uint objectID = tlasInstanceInfo.objectID;
	ObjectInfo objectInfo = ObjectInfo_GetObjectInfo(objectID);

	// MeshInfo
	uint meshID = tlasInstanceInfo.meshID;
 	MeshInfo meshInfo = MeshInfo_GetMeshInfo(meshID);

    // BasicMaterialInfo
    uint materialID = meshInfo.materialID;
    BasicMaterialInfo basicMaterialInfo = BasicMaterialInfo_GetBasicMaterialInfo(materialID);

	// CameraInfo (MainCameraData)
    CameraData cameraData = CameraInfo_GetCameraData(CameraInfo_GetMainCameraIndex());
    mat4 projView = cameraData.projection * cameraData.view;

	// IndexData
	uint i0 = IndexData_GetIndexData(meshInfo.indexDataOffset, tri * 3 + 0);
	uint i1 = IndexData_GetIndexData(meshInfo.indexDataOffset, tri * 3 + 1);
	uint i2 = IndexData_GetIndexData(meshInfo.indexDataOffset, tri * 3 + 2);

	// VertexData
	VertexData v0 = VertexData_GetVertexData(meshInfo.vertexDataOffset, meshInfo.vertexAttributeBit, meshInfo.unitSize, i0);
	VertexData v1 = VertexData_GetVertexData(meshInfo.vertexDataOffset, meshInfo.vertexAttributeBit, meshInfo.unitSize, i1);
	VertexData v2 = VertexData_GetVertexData(meshInfo.vertexDataOffset, meshInfo.vertexAttributeBit, meshInfo.unitSize, i2);

	VertexData interpolateVertex = InterpolateVertexAttributes(v0, v1, v2, bary);

    // マテリアル
    interpolateVertex.color = interpolateVertex.color * basicMaterialInfo.baseColor;
    if (BasicMaterialInfo_HasBaseColorTexture(basicMaterialInfo.bitFlags))
    {
        vec3 baseColorTextureColor = Sampler2DLoader_GetTexture(basicMaterialInfo.baseColorTexture, interpolateVertex.uv0).rgb;
        interpolateVertex.color = interpolateVertex.color * vec4(baseColorTextureColor, 1.0);
    }
	
	// rayPayload.color = CalcLighting(interpolateVertex, objectInfo, cameraData).rgb;
	rayPayload.color = interpolateVertex.color.rgb;
	rayPayload.distance = 0;	// todo
}
