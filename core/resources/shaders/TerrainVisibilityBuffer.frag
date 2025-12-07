#version 450
#extension GL_EXT_debug_printf : enable

layout(location = 0) flat in uint inVBInfoIndex;
// layout(location = 0) out vec4 outFragColor;
layout(location = 0) out uint outVisibilityBuffer;

const uint PRIM_BITS = 7u;                      // 2^7 = 128 > 124
const uint PRIM_MASK = (1u << PRIM_BITS) - 1u;  // 0x7F = 127
const uint VB_BITS = 32u - PRIM_BITS;           // 25bit;
const uint VB_MAX = (1u << VB_BITS) - 1u;       // 0x1FFFFFFF ≒ 3,300万

uint EncodeVisibility(uint vbIndex, uint primIndex)
{
    return (vbIndex << PRIM_BITS) | (primIndex & PRIM_MASK);
}

void main()
{
  uint primIndex = uint(gl_PrimitiveID);

  // マイナスチェック
  if (gl_PrimitiveID < 0)
  {
    // outFragColor = vec4(1.0, 0.0, 1.0, 1.0);
    return;
  }
  else if ((primIndex > PRIM_MASK) || (inVBInfoIndex > VB_MAX))
  {
    // outFragColor = vec4(0.5, 0.5, 0.5, 1.0);
    return;
  }
  
  uint packedVisibility = EncodeVisibility(inVBInfoIndex, primIndex);
  outVisibilityBuffer = packedVisibility + 1; // +1のため、0は無効として扱う
}