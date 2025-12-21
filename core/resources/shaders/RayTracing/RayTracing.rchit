#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require

#include "../Descriptors/ObjectInfo.glsl"
#include "../Descriptors/MeshInfo.glsl"

#include "../Descriptors/VertexData.glsl"
#include "../Descriptors/IndexData.glsl"

layout(location = 0) rayPayloadInEXT vec3 hitValue;
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

void main()
{
	const vec3 bary = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);

	// PrimID
	uint tri = gl_PrimitiveID;

	// ObjectInfo
	uint objectID = gl_InstanceCustomIndexEXT;
	ObjectInfo objectInfo = ObjectInfo_GetObjectInfo(objectID);

      // MeshInfo
      uint meshID = objectInfo.meshID;
      MeshInfo meshInfo = MeshInfo_GetMeshInfo(meshID);

	// IndexData
	uint i0 = IndexData_GetIndexData(meshInfo.indexDataOffset, tri * 3 + 0);
	uint i1 = IndexData_GetIndexData(meshInfo.indexDataOffset, tri * 3 + 1);
	uint i2 = IndexData_GetIndexData(meshInfo.indexDataOffset, tri * 3 + 2);

	// VertexData
	VertexData v0 = VertexData_GetVertexData(meshInfo.vertexDataOffset, meshInfo.vertexAttributeBit, meshInfo.unitSize, i0);
	VertexData v1 = VertexData_GetVertexData(meshInfo.vertexDataOffset, meshInfo.vertexAttributeBit, meshInfo.unitSize, i1);
	VertexData v2 = VertexData_GetVertexData(meshInfo.vertexDataOffset, meshInfo.vertexAttributeBit, meshInfo.unitSize, i2);

	VertexData v = InterpolateVertexAttributes(v0, v1, v2, bary);

  	// hitValue = v.color.rgb;

      // Debug
      hitValue = vec3(uintBitsToFloat(objectID), uintBitsToFloat(objectID), uintBitsToFloat(objectID));
}
