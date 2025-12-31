#version 460
#extension GL_EXT_ray_tracing : enable

struct InRayPayload {
	bool shadowed;
};

layout(location = 1) rayPayloadInEXT InRayPayload inRayPayload;

void main()
{
    inRayPayload.shadowed = false;
}