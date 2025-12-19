#ifndef TLAS
#define TLAS

layout (set = 4, binding = 0) uniform accelerationStructureEXT TLASDescriptor;

accelerationStructureEXT TLAS_GetTLAS()
{
    return TLASDescriptor;
}

#endif