#version 450
#extension GL_GOOGLE_include_directive : require

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV0;
layout (location = 3) in vec2 inUV1;
layout (location = 4) in vec4 inColor;

layout (location = 0) out vec4 outColor;

layout (push_constant) uniform PushConstant {
    ivec2 atlasSize;
    ivec2 offset;
    ivec2 size;
};

void main() 
{
    outColor = inColor;

    vec2 localPixel = vec2(offset) + inUV1 * vec2(size);
    vec2 atlasUV = localPixel / vec2(atlasSize);
    vec2 pos = atlasUV * 2.0 - 1.0;
    gl_Position = vec4(pos, 0.0, 1.0);
}