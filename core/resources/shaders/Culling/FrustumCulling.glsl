#ifndef FRUSTUMCULLING
#define FRUSTUMCULLING

struct OBBData {
    vec4 center;
	vec4 axisX;
	vec4 axisY;
	vec4 axisZ;
};

struct SphereData {
    vec3 center;
    float radius;
};

mat3 FrustumCulling_RotateX(float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return transpose(mat3(
        1, 0, 0,
        0, c, -s,
        0, s,  c
    ));
}

mat3 FrustumCulling_RotateY(float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return transpose(mat3(
         c, 0, s,
         0, 1, 0,
        -s, 0, c
    ));
}

mat3 FrustumCulling_RotateZ(float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return transpose(mat3(
        c, -s, 0,
        s,  c, 0,
        0,  0, 1
    ));
}

OBBData FrustumCulling_CreateOBBData(vec3 aabbMin, vec3 aabbMax, vec3 position, vec3 rotation, vec3 scale)
{
    OBBData obbData;
    vec3 localExtent = (aabbMax - aabbMin) * 0.5;
    vec3 centerLocal = (aabbMax + aabbMin) * 0.5;
    vec3 scaleExtent = localExtent * scale;

    // オイラー角をラジアンに変換
    vec3 rotRad = radians(rotation);

    // 回転マトリクスを作成（XYZ順）
    mat3 rotMat = mat3(
        // CPU側の順番通りで問題ない (MVPみたいに逆にする必要ない)
        FrustumCulling_RotateX(rotRad.x) *
        FrustumCulling_RotateY(rotRad.y) *
        FrustumCulling_RotateZ(rotRad.z) 
    );

    // 各軸ベクトル + extent をwに格納
    obbData.axisX = vec4(rotMat * vec3(1.0, 0.0, 0.0), scaleExtent.x);
    obbData.axisY = vec4(rotMat * vec3(0.0, 1.0, 0.0), scaleExtent.y);
    obbData.axisZ = vec4(rotMat * vec3(0.0, 0.0, 1.0), scaleExtent.z);

    // 中心点：ローカル空間での中心 → スケール → 回転 → 平行移動
    obbData.center = vec4(position + rotMat * (centerLocal * scale), 0.0);

    return obbData;
}

bool FrustumCulling_IsVisible(vec4 frustumPlanes[6], OBBData obbData)
{
    for (int i = 0; i < 6; i++) 
    {
        vec4 plane = frustumPlanes[i];
        // OBB中心から平面までの距離
        float d = dot(plane.xyz, obbData.center.xyz) + plane.w;
        // OBB各軸を平面法線に投影した長さの合計が半径
        float r = 
            abs(dot(plane.xyz, obbData.axisX.xyz)) * obbData.axisX.w +
            abs(dot(plane.xyz, obbData.axisY.xyz)) * obbData.axisY.w +
            abs(dot(plane.xyz, obbData.axisZ.xyz)) * obbData.axisZ.w;
        if ((d + r < 0.0)) 
        {
            return false;
        }
    }
    return true;
}
#endif