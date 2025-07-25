#ifndef SAMPLER2DLOADER
#define SAMPLER2DLOADER
#extension GL_EXT_nonuniform_qualifier : require

layout (set = 0, binding = 3) uniform sampler2D Sampler2D[];

ivec2 Sampler2DLoader_GetTextureSize(uint textureId, int lod) {
    return textureSize(Sampler2D[textureId], lod);
}

vec4 Sampler2DLoader_GetTexture(uint textureId, vec2 offset) {
    return texture(Sampler2D[textureId], offset);
}

#endif