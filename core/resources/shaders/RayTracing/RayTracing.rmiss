#version 460
#extension GL_EXT_ray_tracing : enable

struct RayPayload { 
	vec3 color;
	float distance;
};

layout(location = 0) rayPayloadInEXT RayPayload rayPayload;

void main()
{
    rayPayload.color = vec3(0.0, 0.0, 0.0);
}