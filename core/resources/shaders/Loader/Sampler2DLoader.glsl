#ifndef SAMPLER2DLOADER
#define SAMPLER2DLOADER
#extension GL_EXT_nonuniform_qualifier : require

layout (set = 3, binding = 0) uniform sampler2D Sampler2D[];

ivec2 Sampler2DLoader_GetTextureSize(uint textureId, int lod) {
    return textureSize(Sampler2D[textureId], lod);
}

vec4 Sampler2DLoader_GetTexture(uint textureId, vec2 offset) {
    return texture(Sampler2D[textureId], offset);
}

vec4 Sampler2DLoader_TexelFetch(uint textureId, ivec2 P, int lod)
{
    return texelFetch(Sampler2D[textureId], P, lod);
}

vec4 Sampler2DLoader_TextureLod(uint textureId, vec2 uv, int lod)
{
    return textureLod(Sampler2D[textureId], uv, lod);
}

#endif