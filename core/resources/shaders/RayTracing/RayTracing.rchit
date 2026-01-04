#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require

#include "../Loader/Sampler2DLoader.glsl"

#include "../Descriptors/TLASInstanceInfo.glsl"
#include "../Descriptors/ObjectInfo.glsl"
#include "../Descriptors/MeshInfo.glsl"
#include "../Descriptors/CameraInfo.glsl"
#include "../Descriptors/TLAS.glsl"

#include "../Descriptors/VertexData.glsl"
#include "../Descriptors/IndexData.glsl"
#include "../Descriptors/BasicMaterialInfo.glsl"

struct InRayPayload {
	vec3 color;
	float distance;
};

layout(location = 0) rayPayloadInEXT InRayPayload inRayPayload;
hitAttributeEXT vec2 attribs;

struct RayPayload {
	bool shadowed;
};
layout(location = 1) rayPayloadEXT RayPayload rayPayload;

float stepRandomFloat(inout uint seed) 
{
	seed = seed * 747796405u + 2891336453u;
	uint word = ((seed >> ((seed >> 28u) + 4u)) ^ seed) * 277803737u;
	return float((word >> 22u) ^ word) / 4294967295.0;
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

	// world normalの計算
	mat3 normalMatrix = mat3(transpose(inverse(objectInfo.model)));
	vec3 worldNormal = normalize(normalMatrix * interpolateVertex.normal);

    // マテリアル
    interpolateVertex.color = interpolateVertex.color * basicMaterialInfo.baseColor;
    if (BasicMaterialInfo_HasBaseColorTexture(basicMaterialInfo.bitFlags))
    {
        vec3 baseColorTextureColor = Sampler2DLoader_GetTexture(basicMaterialInfo.baseColorTexture, interpolateVertex.uv0).rgb;
        interpolateVertex.color = interpolateVertex.color * vec4(baseColorTextureColor, 1.0);
    }

    // Lighting計算
    vec3 color = CalcLighting(interpolateVertex, objectInfo, cameraData).rgb;

	// ShadowRay
	vec3 lightDir = normalize(vec3(0.3, -1.0, 0.2));
	vec3 L = -lightDir;
	float tmin = 0.001;
	float tmax = 10000.0;
    // 一次レイ(前の発射)の発射位置(原点) + 発射方向 * 距離(係数)
	// (worldNormal * 0.001) 念のため、少し外に押し出す (自分に当たる時とか…)
    vec3 start = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT + (worldNormal * 0.001);

	// 最初の乱数Seedの作成
	uint seed = uint(gl_LaunchIDEXT.x * 1973 + gl_LaunchIDEXT.y * 9277 + gl_PrimitiveID * 26699);
	
	float shadowFactor = 0.0;		// 影(係数)
	float shadowHitCount = 0.0;		// PCF用 (最後の結果を平均化)

	// 面が光に向いてるかどうかの判定
	float NdotL = dot(worldNormal, L);
	if (NdotL > 0.0)
    {
		const int SAMPLES = 4;		  	// サンプル数 (増やすと綺麗だけど重くなる…)
		const float spread = 0.05;	  	// 影のボケ具合

		for(int i = 0; i < SAMPLES; i++) 
		{
			// 光の方向をランダムに散らす
			vec3 noiseVec = vec3(stepRandomFloat(seed)-0.5, stepRandomFloat(seed)-0.5, stepRandomFloat(seed)-0.5) * spread;
			vec3 rayDir = normalize(L + noiseVec);

			rayPayload.shadowed = true;
			traceRayEXT(TLAS_GetTLAS(), gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT | gl_RayFlagsCullBackFacingTrianglesEXT, 0xff, 0, 0, 1, start, tmin, rayDir, tmax, 1);

			if (rayPayload.shadowed) shadowHitCount += 1.0;
		}

		// 影の平均値 (0.0: 光 1.0: 影)
		shadowFactor = shadowHitCount / float(SAMPLES);
	}
	else
	{
		// 光に向いてないので、そのまま100% 影でOK
		shadowFactor = 1.0;
	}
	
	// Raytrace Shadow Terminator Problem 対策
	// N.Lが低い境界を滑らかに
	// dot(N・L)の結果が0.1 (ちょっとだけ光に向いてる状態)で、0.1 * 10.0 = 1.0 になる
	// 境界線ギリギリまでは影を100%出す、本当にギリギリのところ(0.1以下)で一気に影を消していくよ
	// 急カーブができる
	const float terminatorOffset = 5.0;
	float terminatorFade = clamp(dot(worldNormal, L) * terminatorOffset, 0.0, 1.0);

	// 最終色の計算
	color *= mix(1.0, 0.7, shadowFactor * terminatorFade);

	// 光に向いてない面に追加の減衰
	if (NdotL <= 0.0)
	{
		color *= 0.5;
	}

	inRayPayload.color = color;
	inRayPayload.distance = gl_HitTEXT;
}