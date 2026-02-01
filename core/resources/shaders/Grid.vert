#version 450

layout (location = 0) out vec3 outNearPoint;
layout (location = 1) out vec3 outFarPoint;

vec2 positions[3] = vec2[](
    vec2(-1.0, -1.0),
    vec2( 3.0, -1.0),
    vec2(-1.0,  3.0)
);

layout(push_constant) uniform PushConstant {
    mat4 invVP;
};

void main() 
{
    vec2 pos = positions[gl_VertexIndex];
    
    // NDC座標からワールド座標へ逆投影
    vec4 n = invVP * vec4(pos, 0.0, 1.0);
    vec4 f = invVP * vec4(pos, 1.0, 1.0);
    
    outNearPoint = n.xyz / n.w;
    outFarPoint = f.xyz / f.w;
    
    gl_Position = vec4(pos, 0.0, 1.0);
}