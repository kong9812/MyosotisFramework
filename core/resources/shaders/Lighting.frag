#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout (set = 1, input_attachment_index = 0, binding = 0) uniform subpassInput inputPosition;
layout (set = 1, input_attachment_index = 1, binding = 1) uniform subpassInput inputNormal;
layout (set = 1, input_attachment_index = 2, binding = 2) uniform subpassInput inputBaseColor;

// Meta情報構造体
struct BaseObjectData {
    uint typeID;        // 今は使わない
    uint dataOffset;    // 実データのoffset
};

// Meta情報テーブル
layout(std430, binding = 0) buffer MetaBuffer {
    BaseObjectData objectTable[];
};

// 実データ
layout(std430, binding = 1) buffer AllDataBuffer {
    uint rawData[]; // 全てのデータをここにまとめる
};

// 画像
layout (binding = 2) uniform sampler2D Sampler2D[];

layout (push_constant) uniform PushConstant {
    uint objectIndex;
    uint textureId;
};

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

float PCFShadow(vec3 shadowCoord, int pcfCount) 
{
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(Sampler2D[textureId], 0);
	uint count = 0; 
    for (int x = -pcfCount; x <= pcfCount; ++x) 
    {
        for (int y = -pcfCount; y <= pcfCount; ++y) 
        {
            vec2 offset = vec2(x, y) * texelSize;
            float closestDepth = texture(Sampler2D[textureId], shadowCoord.xy + offset).r;
            shadow += (shadowCoord.z > closestDepth) ? 1.0 : 0.1;
			count++;
        }
    }
    return shadow / count;
}

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

const vec3 diffuseColor = vec3(1.0);
const vec3 ambientColor = vec3(1.0);
const vec3 specularColor = vec3(1.0);

mat4 LoadMat4(uint base) {
    return mat4(
        vec4(uintBitsToFloat(rawData[base +  0]), uintBitsToFloat(rawData[base +  1]),
             uintBitsToFloat(rawData[base +  2]), uintBitsToFloat(rawData[base +  3])),
        vec4(uintBitsToFloat(rawData[base +  4]), uintBitsToFloat(rawData[base +  5]),
             uintBitsToFloat(rawData[base +  6]), uintBitsToFloat(rawData[base +  7])),
        vec4(uintBitsToFloat(rawData[base +  8]), uintBitsToFloat(rawData[base +  9]),
             uintBitsToFloat(rawData[base + 10]), uintBitsToFloat(rawData[base + 11])),
        vec4(uintBitsToFloat(rawData[base + 12]), uintBitsToFloat(rawData[base + 13]),
             uintBitsToFloat(rawData[base + 14]), uintBitsToFloat(rawData[base + 15]))
    );
}
vec4 LoadVec4(uint base) {
    return vec4(
        uintBitsToFloat(rawData[base + 0]),
        uintBitsToFloat(rawData[base + 1]),
        uintBitsToFloat(rawData[base + 2]),
        uintBitsToFloat(rawData[base + 3])
    );
}
void main() 
{
    BaseObjectData meta = objectTable[nonuniformEXT(objectIndex)];

    vec4 cameraPosition = LoadVec4(meta.dataOffset + 0);
    mat4 viewProjection = LoadMat4(meta.dataOffset + 4);
    vec4 lightPosition = LoadVec4(meta.dataOffset + 20);
    int pcfCount = int(rawData[meta.dataOffset + 24]);

	vec4 position = subpassLoad(inputPosition);
    vec4 normal = subpassLoad(inputNormal);
	vec4 baseColor = subpassLoad(inputBaseColor);

	vec4 lightSpace = biasMat * viewProjection * position;
	float shadow = PCFShadow(vec3((lightSpace / lightSpace.w).xyz), pcfCount);

	vec3 lightDir = normalize(lightPosition.xyz - position.xyz);
    vec3 reflectDir = reflect(-lightDir, normal.xyz);
    vec3 viewDir = normalize(cameraPosition.xyz - position.xyz);
    vec3 halfwayDir = normalize(lightDir + cameraPosition.xyz);

    float diff = max(dot(normal.xyz, lightDir), 0.0);
    float spec = pow(max(dot(normal.xyz, halfwayDir), 0.0), 64.0);

	vec3 ambient = 0.5 * ambientColor;
    vec3 diffuse = diff * diffuseColor;
    vec3 specular = vec3(0.0);

	vec3 color = (ambient + (1.0 - shadow) * (diffuse + specular)) * baseColor.xyz;
	outColor = vec4(color, baseColor.a);
}