#ifndef MAINCAMERADATALOADER
#define MAINCAMERADATALOADER
#extension GL_EXT_nonuniform_qualifier : require

struct MainCameraData {
    vec4 frustumPlanes[6];
    mat4 view;
    mat4 projection;
    vec4 position;
};

layout (set = 0, binding = 0) uniform MainCameraBuffer {
    MainCameraData cameraData;
};

MainCameraData MainCameraDataLoader_GetMainCameraData() {
    return cameraData;
}
#endif