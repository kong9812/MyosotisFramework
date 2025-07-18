#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(binding = 1) uniform sampler2D Sampler2D[];

layout(push_constant) uniform PushConstant {
    uint textureId;
    uint bufferId;
	vec2 screenSize;
};

layout (location = 0) out vec4 outColor;

void main() 
{
	outColor = texture(Sampler2D[textureId], gl_FragCoord.xy / screenSize);
}