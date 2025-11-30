#ifndef BACKFACECULLING
#define BACKFACECULLING

//ref: https://www.youtube.com/watch?v=h_Aqol0oTs4
bool BackfaceCulling_IsVisible(vec3 cameraPosition, vec3 v0, vec3 v1, vec3 v2)
{
    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;

    // 法線ベクトルを計算
    vec3 faceNormal = cross(edge1, edge2);
    // 視線方向ベクトル（v0からカメラへの方向）
    vec3 viewDir = normalize(cameraPosition - v0);

    // 内積で面がカメラ方向を向いているか判定
    return dot(faceNormal, viewDir) > 0.0;
}
#endif