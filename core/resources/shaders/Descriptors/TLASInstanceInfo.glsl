#ifndef TLASINSTANCEINFO
#define TLASINSTANCEINFO

struct TLASInstanceInfo {
	uint objectID;
	uint meshID;
	uint p1;
	uint p2;
};

layout (std430, set = 4, binding = 1) readonly buffer TLASInstanceInfoDescriptor {
    TLASInstanceInfo tlasInstanceInfo[];
};

TLASInstanceInfo TLASInstanceInfo_GetTLASInstanceInfo(uint index)
{
    return tlasInstanceInfo[index];
}

#endif