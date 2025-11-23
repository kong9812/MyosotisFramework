#version 450
// #extension GL_EXT_debug_printf : enable

layout (location = 0) in VertexInput {
  vec4 color;
} vertexInput;

layout(location = 0) out vec4 outFragColor;

void main()
{
  // debugPrintfEXT("Hello from frag shader!\n");
	outFragColor = vertexInput.color;
}