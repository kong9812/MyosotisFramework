# Copyright (c) 2025 kong9812
set(DLL_PROJECT_NAME "MyosotisDLL")

FILE(GLOB_RECURSE DLL_SOURCES 
  "${PROJECT_SOURCE_DIR}/frameworkDLL/include/*.h"
  "${PROJECT_SOURCE_DIR}/frameworkDLL/src/*.cpp"
)
GroupSourcesByFolder(${DLL_SOURCES})

add_library(${DLL_PROJECT_NAME} SHARED ${DLL_SOURCES} ${APP_INFO_RC})
target_link_libraries(${DLL_PROJECT_NAME} PRIVATE ${FRAMEWORK_PROJECT_NAME})

AddIncludeDirectories(${DLL_PROJECT_NAME} ${DLL_SOURCES})
AddIncludeDirectories(${DLL_PROJECT_NAME} ${FRAMEWORK_SOURCES})

# glfw
target_link_libraries(${DLL_PROJECT_NAME} PRIVATE glfw)
# glm
target_link_libraries(${DLL_PROJECT_NAME} PRIVATE glm::glm)
# ofbx
source_group("external/ofbx" FILES ${OFBX_SOURCES})
target_sources(${DLL_PROJECT_NAME} PRIVATE ${OFBX_SOURCES})
target_include_directories(${DLL_PROJECT_NAME} PRIVATE ${ofbx_SOURCES_DIR}/src)
# imgui
source_group("external/imgui" FILES ${IMGUI_SOURCES})
target_sources(${DLL_PROJECT_NAME} PRIVATE ${IMGUI_SOURCES})
target_include_directories(${DLL_PROJECT_NAME} PRIVATE ${imgui_SOURCES_DIR})
# vma
source_group("external/vma" FILES ${VMA_SOURCES})
target_sources(${DLL_PROJECT_NAME} PRIVATE ${VMA_SOURCES})
target_include_directories(${DLL_PROJECT_NAME} PRIVATE ${vma_SOURCE_DIR}/include/)
# RapidJSON
target_include_directories(${DLL_PROJECT_NAME} PRIVATE ${RapidJSON_SOURCE_DIR}/include)
# stduuid
target_include_directories(${DLL_PROJECT_NAME} PRIVATE ${stduuid_SOURCE_DIR})
target_include_directories(${DLL_PROJECT_NAME} PRIVATE ${stduuid_SOURCE_DIR}/include)
# stb
source_group("external/stb" FILES ${STB_SOURCES})
target_sources(${DLL_PROJECT_NAME} PRIVATE ${STB_SOURCES})
target_include_directories(${DLL_PROJECT_NAME} PRIVATE ${stb_SOURCE_DIR})
# vulkan
find_package(Vulkan REQUIRED)
target_include_directories(${DLL_PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${DLL_PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES})

# ビルド構成ごとに出力ディレクトリ設定
set_target_properties(${DLL_PROJECT_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}
  RUNTIME_OUTPUT_DIRECTORY_FWDLL ${PROJECT_SOURCE_DIR}
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}
)

# build switch
target_compile_definitions(${DLL_PROJECT_NAME} PRIVATE
  $<$<CONFIG:Debug>:DEBUG>
  $<$<CONFIG:FWDLL>:FWDLL>
  $<$<CONFIG:Release>:RELEASE>
)