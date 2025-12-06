#ifndef USAMPLER2DLOADER
#define USAMPLER2DLOADER
#extension GL_EXT_nonuniform_qualifier : require

layout (set = 3, binding = 0) uniform usampler2D Usampler2D[];

ivec2 Usampler2DLoader_GetTextureSize(uint textureId, int lod) {
    return textureSize(Usampler2D[textureId], lod);
}

uvec4 Usampler2DLoader_GetTexture(uint textureId, vec2 offset) {
    return texture(Usampler2D[textureId], offset);
}

uvec4 Usampler2DLoader_TexelFetch(uint textureId, ivec2 P, int lod)
{
    return texelFetch(Usampler2D[textureId], P, lod);
}

uvec4 Usampler2DLoader_TextureLod(uint textureId, vec2 uv, int lod)
{
    return textureLod(Usampler2D[textureId], uv, lod);
}

#endif