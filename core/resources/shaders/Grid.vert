#version 450
layout (location = 0) out vec2 outNDC;

const vec2 positions[3] = vec2[](
    vec2(-1.0, -1.0),   // 左下
    vec2( 3.0, -1.0),   // 右下
    vec2(-1.0,  3.0)    // 左上
);

layout(push_constant) uniform PushConstant {
    mat4 invVP;
};

void main() 
{
    outNDC = positions[gl_VertexIndex];    
    gl_Position = vec4(outNDC, 0.0, 1.0);
}