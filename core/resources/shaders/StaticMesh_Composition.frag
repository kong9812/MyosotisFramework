#version 450

layout (input_attachment_index = 0, binding = 0) uniform subpassInput inputPosition;
layout (input_attachment_index = 1, binding = 1) uniform subpassInput inputBaseColor;

layout (location = 0) out vec4 outColor;

void main() 
{
	vec4 baseColor = subpassLoad(inputBaseColor);
	outColor = baseColor;
}