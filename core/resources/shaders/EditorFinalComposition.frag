#version 450

layout (set = 1, input_attachment_index = 0, binding = 0) uniform subpassInput mainRenderTarget;
layout (set = 1, input_attachment_index = 1, binding = 1) uniform subpassInput editorRenderTarget;

layout (location = 0) out vec4 outColor;

void main() 
{
	vec4 mainRenderTargetColor = subpassLoad(mainRenderTarget);
	vec4 editorRenderTargetColor = subpassLoad(editorRenderTarget);
	outColor = mainRenderTargetColor + editorRenderTargetColor;
}