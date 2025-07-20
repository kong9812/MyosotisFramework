#version 450

layout (set = 1, input_attachment_index = 0, binding = 0) uniform subpassInput offscreen;

layout (location = 0) out vec4 outColor;

void main() 
{
	vec4 offscreenColor = subpassLoad(offscreen);
	outColor = offscreenColor;
}