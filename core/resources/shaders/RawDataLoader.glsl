// Meta情報構造体
struct BaseObjectData {
    uint typeID;        // 今は使わない
    uint dataOffset;    // 実データのoffset
};

// Meta情報テーブル
layout(std430, binding = 0) readonly buffer MetaBuffer {
    BaseObjectData objectTable[];
};

// 実データ
layout(std430, binding = 1) readonly buffer AllDataBuffer {
    uint rawData[]; // 全てのデータをここにまとめる
};

mat4 LoadMat4(uint base) {
    return mat4(
        vec4(uintBitsToFloat(rawData[base +  0]), uintBitsToFloat(rawData[base +  1]),
             uintBitsToFloat(rawData[base +  2]), uintBitsToFloat(rawData[base +  3])),
        vec4(uintBitsToFloat(rawData[base +  4]), uintBitsToFloat(rawData[base +  5]),
             uintBitsToFloat(rawData[base +  6]), uintBitsToFloat(rawData[base +  7])),
        vec4(uintBitsToFloat(rawData[base +  8]), uintBitsToFloat(rawData[base +  9]),
             uintBitsToFloat(rawData[base + 10]), uintBitsToFloat(rawData[base + 11])),
        vec4(uintBitsToFloat(rawData[base + 12]), uintBitsToFloat(rawData[base + 13]),
             uintBitsToFloat(rawData[base + 14]), uintBitsToFloat(rawData[base + 15]))
    );
}

vec4 LoadVec4(uint base) {
    return vec4(
        uintBitsToFloat(rawData[base + 0]),
        uintBitsToFloat(rawData[base + 1]),
        uintBitsToFloat(rawData[base + 2]),
        uintBitsToFloat(rawData[base + 3])
    );
}

int LoadInt(uint base) {
    return int(rawData[base]);
}