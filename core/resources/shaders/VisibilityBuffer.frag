#version 450
#extension GL_EXT_debug_printf : enable

layout(location = 0) flat in uint inVBInfoIndex;
layout(location = 0) out vec4 outFragColor;

const uint PRIM_BITS = 7u;                      // 2^7 = 128 > 124
const uint PRIM_MASK = (1u << PRIM_BITS) - 1u;  // 0x7F = 127
const uint VB_BITS = 32u - PRIM_BITS;           // 25bit;
const uint VB_MAX = (1u << VB_BITS) - 1u;       // 0x1FFFFFFF ≒ 3,300万

// ハッシュ関数（uint版）
uint hash(uint x) {
    // 32-bit integer hash based on PCG
    x = x * 747796405u + 2891336453u;
    x = ((x >> ((x >> 28u) + 4u)) ^ x) * 277803737u;
    return (x >> 22u) ^ x;
}

// uintのシード値を受け取り vec4 カラーを返す関数
vec4 randomColor_uint(uint seed)
{
    // 1. シードをハッシュ化してランダム性を高める
    uint hash_r = hash(seed);
    uint hash_g = hash(seed + 1u); // +1u でシードを変えて独立した色成分を生成
    uint hash_b = hash(seed + 2u);

    // 2. 32-bit uint を 0.0 ~ 1.0 の float に変換する
    //    uint の最大値 (0xFFFFFFFFu) で割ることで正規化
    //    (注: `uintBitsToFloat` の方が高速ですが、色に変換するには正規化が必要です)
    const float max_uint = float(0xFFFFFFFFu);
    
    float r = float(hash_r) / max_uint;
    float g = float(hash_g) / max_uint;
    float b = float(hash_b) / max_uint;

    return vec4(r, g, b, 1.0);
}

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
    outFragColor = vec4(1.0, 0.0, 1.0, 1.0);
    return;
  }
  else if ((primIndex > PRIM_MASK) || (inVBInfoIndex > VB_MAX))
  {
    outFragColor = vec4(0.5, 0.5, 0.5, 1.0);
    return;
  }
  
  uint packedVisibility = EncodeVisibility(inVBInfoIndex, primIndex);
	outFragColor = randomColor_uint(packedVisibility);
}