# Copyright (c) 2025 kong9812
cmake_minimum_required(VERSION 3.22)
cmake_policy(SET CMP0091 NEW)

include(FetchContent)

project(MyosotisFW)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(DLL_PROJECT_NAME "myosotis")
set(CMAKE_CONFIGURATION_TYPES "Debug;FWDLL;Release" CACHE STRING "" FORCE)

set(APP_INFO_RC ${PROJECT_SOURCE_DIR}/resources/appInfo/appInfo.rc)

FILE(GLOB_RECURSE SOURCES 
  "${PROJECT_SOURCE_DIR}/include/*.h"
  "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

# Ref: https://github.com/TheLartians/GroupSourcesByFolder.cmake
# Thank you :)
function(GroupSourcesByFolder target)
  set(SOURCE_GROUP_DELIMITER "/")
  set(last_dir "")
  set(files "")
  set(include_dirs "")

  get_target_property(sources ${target} SOURCES)

  foreach(file ${sources})
    file(RELATIVE_PATH relative_file "${PROJECT_SOURCE_DIR}" ${file})
    get_filename_component(dir "${relative_file}" PATH)
    if(NOT "${dir}" STREQUAL "${last_dir}")
      if(files)
        source_group("${last_dir}" FILES ${files})
      endif()
      set(files "")
    endif()
    set(files ${files} ${file})
    set(last_dir "${dir}")

    get_filename_component(extension "${file}" EXT)
    if(extension STREQUAL ".h")
      # ヘッダーファイルの場合、インクルードディレクトリに追加
      list(APPEND include_dirs "${dir}")
    endif()
  endforeach()

  if(files)
    source_group("${last_dir}" FILES ${files})
  endif()

  # ターゲットのインクルードディレクトリに追加
  target_include_directories(${target} PRIVATE ${include_dirs})
endfunction()

# 実行プロジェクトの作成
add_executable(${PROJECT_NAME} ${SOURCES} ${APP_INFO_RC})

# ライブラリの作成
add_library(${DLL_PROJECT_NAME} SHARED ${SOURCES})
GroupSourcesByFolder(${PROJECT_NAME})
GroupSourcesByFolder(${DLL_PROJECT_NAME})

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
target_link_libraries(${DLL_PROJECT_NAME} PRIVATE glfw)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glm)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)
target_link_libraries(${DLL_PROJECT_NAME} PRIVATE glm::glm)

# ofbx
FetchContent_Declare(
   ofbx
   GIT_REPOSITORY https://github.com/nem0/OpenFBX.git
   GIT_TAG master
)
FetchContent_MakeAvailable(ofbx)
set(ofbx_SOURCES_DIR ${FETCHCONTENT_BASE_DIR}/ofbx-src)
set(OFBX_SOURCES
  ${ofbx_SOURCES_DIR}/src/libdeflate.c
  ${ofbx_SOURCES_DIR}/src/libdeflate.h
  ${ofbx_SOURCES_DIR}/src/ofbx.h
  ${ofbx_SOURCES_DIR}/src/ofbx.cpp
)
source_group("external/ofbx" FILES ${OFBX_SOURCES})
target_sources(${PROJECT_NAME} PRIVATE ${OFBX_SOURCES})
target_sources(${DLL_PROJECT_NAME} PRIVATE ${OFBX_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${ofbx_SOURCES_DIR}/src)
target_include_directories(${DLL_PROJECT_NAME} PRIVATE ${ofbx_SOURCES_DIR}/src)

# imgui
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.91.7
)
FetchContent_MakeAvailable(imgui)
set(imgui_SOURCES_DIR "${FETCHCONTENT_BASE_DIR}/imgui-src")
set(IMGUI_SOURCES
  ${imgui_SOURCES_DIR}/imgui.cpp
  ${imgui_SOURCES_DIR}/imgui_draw.cpp
  ${imgui_SOURCES_DIR}/imgui_tables.cpp
  ${imgui_SOURCES_DIR}/imgui_widgets.cpp
  
  ${imgui_SOURCES_DIR}/imgui_demo.cpp

  ${imgui_SOURCES_DIR}/imconfig.h
  ${imgui_SOURCES_DIR}/imgui.h
  ${imgui_SOURCES_DIR}/imstb_rectpack.h
  ${imgui_SOURCES_DIR}/imstb_textedit.h
  ${imgui_SOURCES_DIR}/imstb_truetype.h

  ${imgui_SOURCES_DIR}/backends/imgui_impl_vulkan.cpp
  ${imgui_SOURCES_DIR}/backends/imgui_impl_glfw.cpp

  ${imgui_SOURCES_DIR}/backends/imgui_impl_vulkan.h
  ${imgui_SOURCES_DIR}/backends/imgui_impl_glfw.h
)
source_group("external/imgui" FILES ${IMGUI_SOURCES})
target_sources(${PROJECT_NAME} PRIVATE ${IMGUI_SOURCES})
target_sources(${DLL_PROJECT_NAME} PRIVATE ${IMGUI_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${imgui_SOURCES_DIR})
target_include_directories(${DLL_PROJECT_NAME} PRIVATE ${imgui_SOURCES_DIR})

# VMA(VulkanMemoryAllocator)
FetchContent_Declare(
  vma
  GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
  GIT_TAG v3.2.0
)
set(VMA_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(VMA_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(vma_SOURCE_DIR ${FETCHCONTENT_BASE_DIR}/vma-src)
FetchContent_MakeAvailable(vma)
set(VMA_SOURCES
  ${vma_SOURCE_DIR}/include/vk_mem_alloc.h
  ${vma_SOURCE_DIR}/src/Common.h
  ${vma_SOURCE_DIR}/src/VmaUsage.h

  ${vma_SOURCE_DIR}/src/Common.cpp
  ${vma_SOURCE_DIR}/src/VmaUsage.cpp
)
source_group("external/vma" FILES ${VMA_SOURCES})
target_sources(${PROJECT_NAME} PRIVATE ${VMA_SOURCES})
target_sources(${DLL_PROJECT_NAME} PRIVATE ${VMA_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${vma_SOURCE_DIR}/include/)
target_include_directories(${DLL_PROJECT_NAME} PRIVATE ${vma_SOURCE_DIR}/include/)

# RapidJSON
FetchContent_Declare(
  RapidJSON
  GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
  GIT_TAG d621dc9e9c77f81e5c8a35b8dcc16dcd63351321
)
set(RAPIDJSON_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(RAPIDJSON_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(RAPIDJSON_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(RapidJSON)
target_include_directories(${PROJECT_NAME} PRIVATE ${RapidJSON_SOURCE_DIR}/include)
target_include_directories(${DLL_PROJECT_NAME} PRIVATE ${RapidJSON_SOURCE_DIR}/include)

# stduuid
FetchContent_Declare(
    stduuid
    GIT_REPOSITORY https://github.com/mariusbancila/stduuid.git
    GIT_TAG v1.2.3
)
set(UUID_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(UUID_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(stduuid)
target_include_directories(${PROJECT_NAME} PRIVATE ${stduuid_SOURCE_DIR})
target_include_directories(${DLL_PROJECT_NAME} PRIVATE ${stduuid_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${stduuid_SOURCE_DIR}/include)
target_include_directories(${DLL_PROJECT_NAME} PRIVATE ${stduuid_SOURCE_DIR}/include)

# stb
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)
FetchContent_MakeAvailable(stb)
set(STB_SOURCES
  ${stb_SOURCE_DIR}/stb_image.h
)
source_group("external/stb" FILES ${STB_SOURCES})
target_sources(${PROJECT_NAME} PRIVATE ${STB_SOURCES})
target_sources(${DLL_PROJECT_NAME} PRIVATE ${STB_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${stb_SOURCE_DIR})
target_include_directories(${DLL_PROJECT_NAME} PRIVATE ${stb_SOURCE_DIR})

# Vulkan
find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES})
target_include_directories(${DLL_PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${DLL_PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES})

# work directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
)

# FwDLL構成用にデバッグ情報を生成し、最適化を無効化する
set(CMAKE_C_FLAGS_FWDLL "${CMAKE_C_FLAGS_DEBUG}" CACHE STRING "C flags for FwDLL configuration")
set(CMAKE_CXX_FLAGS_FWDLL "${CMAKE_CXX_FLAGS_DEBUG}" CACHE STRING "C++ flags for FwDLL configuration")
set(CMAKE_EXE_LINKER_FLAGS_FWDLL "${CMAKE_EXE_LINKER_FLAGS_DEBUG}" CACHE STRING "Linker flags for FwDLL configuration")
set(CMAKE_SHARED_LINKER_FLAGS_FWDLL "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}" CACHE STRING "Shared linker flags for FwDLL configuration")
# FwDLL構成にデバッグ用のフラグを設定
set_target_properties(${PROJECT_NAME} PROPERTIES
    COMPILE_FLAGS_FWDLL "/Zi /Od"       # デバッグ情報生成と最適化無効化
    LINK_FLAGS_FWDLL "/DEBUG"           # リンカにデバッグ情報を生成
)

# pdbの生成を抑制するため
set_target_properties(${DLL_PROJECT_NAME} PROPERTIES
        COMPILE_FLAGS "/Zl"
        LINK_FLAGS "/DEBUG:NONE"
    )

# build switch
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:RELEASE>
    $<$<CONFIG:FWDLL>:FWDLL>
)
target_compile_definitions(${DLL_PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:RELEASE>
    $<$<CONFIG:FWDLL>:FWDLL>
)

# GLFW
set_target_properties(glfw PROPERTIES FOLDER "external")
set_target_properties(update_mappings PROPERTIES FOLDER "external")
# glm
set_target_properties(glm PROPERTIES FOLDER "external")
# travis_doc
set_target_properties(travis_doc PROPERTIES FOLDER "external")
# ofbx
set_target_properties(libdeflate_prog_utils PROPERTIES FOLDER "external")
set_target_properties(libdeflate_shared PROPERTIES FOLDER "external")
set_target_properties(libdeflate-gzip PROPERTIES FOLDER "external")
set_target_properties(libdeflate_static PROPERTIES FOLDER "external")
set_target_properties(OpenFBX PROPERTIES FOLDER "external")
