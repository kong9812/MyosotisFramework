#version 450
#extension GL_GOOGLE_include_directive : require

void main() 
{
    // Fullscreen Triangle
    vec2 grid = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(grid * 2.0 - 1.0, 0.0, 1.0);
}