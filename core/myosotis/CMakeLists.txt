# Copyright (c) 2025 kong9812
set(GAME_PROJECT_NAME "Myosotis")

FILE(GLOB_RECURSE GAME_SOURCES 
  "${PROJECT_SOURCE_DIR}/myosotis/include/*.h"
  "${PROJECT_SOURCE_DIR}/myosotis/src/*.cpp"
)
GroupSourcesByFolder(${GAME_SOURCES})

add_executable(${GAME_PROJECT_NAME} ${GAME_SOURCES} ${APP_INFO_RC})
target_link_libraries(${GAME_PROJECT_NAME} PRIVATE ${FRAMEWORK_PROJECT_NAME})

AddIncludeDirectories(${GAME_PROJECT_NAME} ${GAME_SOURCES})
AddIncludeDirectories(${GAME_PROJECT_NAME} ${FRAMEWORK_SOURCES})

# glfw
target_link_libraries(${GAME_PROJECT_NAME} PRIVATE glfw)
# glm
target_link_libraries(${GAME_PROJECT_NAME} PRIVATE glm::glm)
# ofbx
source_group("external/ofbx" FILES ${OFBX_SOURCES})
target_sources(${GAME_PROJECT_NAME} PRIVATE ${OFBX_SOURCES})
target_include_directories(${GAME_PROJECT_NAME} PRIVATE ${ofbx_SOURCES_DIR}/src)
# imgui
source_group("external/imgui" FILES ${IMGUI_SOURCES})
target_sources(${GAME_PROJECT_NAME} PRIVATE ${IMGUI_SOURCES})
target_include_directories(${GAME_PROJECT_NAME} PRIVATE ${imgui_SOURCES_DIR})
# vma
source_group("external/vma" FILES ${VMA_SOURCES})
target_sources(${GAME_PROJECT_NAME} PRIVATE ${VMA_SOURCES})
target_include_directories(${GAME_PROJECT_NAME} PRIVATE ${vma_SOURCE_DIR}/include/)
# RapidJSON
target_include_directories(${GAME_PROJECT_NAME} PRIVATE ${RapidJSON_SOURCE_DIR}/include)
# stduuid
target_include_directories(${GAME_PROJECT_NAME} PRIVATE ${stduuid_SOURCE_DIR})
target_include_directories(${GAME_PROJECT_NAME} PRIVATE ${stduuid_SOURCE_DIR}/include)
# stb
source_group("external/stb" FILES ${STB_SOURCES})
target_sources(${GAME_PROJECT_NAME} PRIVATE ${STB_SOURCES})
target_include_directories(${GAME_PROJECT_NAME} PRIVATE ${stb_SOURCE_DIR})
# tinygltf
target_include_directories(${GAME_PROJECT_NAME} PRIVATE ${tinygltf_SOURCE_DIR})
# vulkan
find_package(Vulkan REQUIRED)
target_include_directories(${GAME_PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${GAME_PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES})
# meshoptimizer
target_sources(${GAME_PROJECT_NAME} PRIVATE ${meshoptimizer_SOURCES})
target_include_directories(${GAME_PROJECT_NAME} PRIVATE ${meshoptimizer_SOURCE_DIR}/src)
# xatlas
target_sources(${GAME_PROJECT_NAME} PRIVATE ${xatlas_SOURCES})
target_include_directories(${GAME_PROJECT_NAME} PRIVATE ${xatlas_SOURCE_DIR})

# work directory
set_target_properties(${GAME_PROJECT_NAME} PROPERTIES
  VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
)

# build switch
target_compile_definitions(${GAME_PROJECT_NAME} PRIVATE
  NOMINMAX
  $<$<CONFIG:Debug>:DEBUG>
  $<$<CONFIG:FWDLL>:FWDLL>
  $<$<CONFIG:Release>:RELEASE>
)

# FWDLL構成にデバッグ用のフラグを設定
target_compile_options(${GAME_PROJECT_NAME} PRIVATE 
  $<$<CONFIG:FWDLL>:/Zi>  # Debug構成に /Zi を指定
)
target_link_options(${GAME_PROJECT_NAME} PRIVATE 
  $<$<CONFIG:FWDLL>:/DEBUG>
)