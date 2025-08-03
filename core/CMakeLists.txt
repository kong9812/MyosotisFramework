# Copyright (c) 2025 kong9812
cmake_minimum_required(VERSION 3.22)
cmake_policy(SET CMP0091 NEW)

include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)

project(MyosotisEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CONFIGURATION_TYPES "Debug;FWDLL;Release" CACHE STRING "" FORCE)
set(APP_INFO_RC ${PROJECT_SOURCE_DIR}/resources/appInfo/appInfo.rc)
set(CMAKE_AUTOMOC ON)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# FwDLL構成用にデバッグ情報を生成し、最適化を無効化する
set(CMAKE_C_FLAGS_FWDLL "${CMAKE_C_FLAGS_DEBUG}" CACHE STRING "C flags for FwDLL configuration")
set(CMAKE_CXX_FLAGS_FWDLL "${CMAKE_CXX_FLAGS_DEBUG}" CACHE STRING "C++ flags for FwDLL configuration")
set(CMAKE_EXE_LINKER_FLAGS_FWDLL "${CMAKE_EXE_LINKER_FLAGS_DEBUG}" CACHE STRING "Linker flags for FwDLL configuration")
set(CMAKE_SHARED_LINKER_FLAGS_FWDLL "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}" CACHE STRING "Shared linker flags for FwDLL configuration")

# ソースファイルをフォルダごとに整理する関数
function(GroupSourcesByFolder)
  set(SOURCE_GROUP_DELIMITER "/")
  set(last_dir "")
  set(files "")
  set(sources ${ARGN})
  
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
  endforeach()

  if(files)
    source_group("${last_dir}" FILES ${files})
  endif()
endfunction()

# ヘッダーファイルのディレクタをターゲットに追加する関数
function(AddIncludeDirectories target)
  set(include_dirs "")
  set(sources ${ARGN})

  foreach(file ${sources})
    get_filename_component(extension "${file}" EXT)
    if(extension STREQUAL ".h")
      get_filename_component(dir "${file}" PATH)
      list(APPEND include_dirs "${dir}")
    endif()
  endforeach()

  target_include_directories(${target} PRIVATE ${include_dirs})
endfunction()

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
set_target_properties(glfw PROPERTIES FOLDER "external")
set_target_properties(update_mappings PROPERTIES FOLDER "external")

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glm)
set_target_properties(glm PROPERTIES FOLDER "external")

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
set_target_properties(libdeflate_prog_utils PROPERTIES FOLDER "external")
set_target_properties(libdeflate_shared PROPERTIES FOLDER "external")
set_target_properties(libdeflate-gzip PROPERTIES FOLDER "external")
set_target_properties(libdeflate_static PROPERTIES FOLDER "external")
set_target_properties(OpenFBX PROPERTIES FOLDER "external")

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
  ${imgui_SOURCES_DIR}/imgui_internal.h
  ${imgui_SOURCES_DIR}/imstb_rectpack.h
  ${imgui_SOURCES_DIR}/imstb_textedit.h
  ${imgui_SOURCES_DIR}/imstb_truetype.h

  ${imgui_SOURCES_DIR}/backends/imgui_impl_vulkan.cpp
  ${imgui_SOURCES_DIR}/backends/imgui_impl_win32.cpp
  # ${imgui_SOURCES_DIR}/backends/imgui_impl_glfw.cpp

  ${imgui_SOURCES_DIR}/backends/imgui_impl_vulkan.h
  ${imgui_SOURCES_DIR}/backends/imgui_impl_win32.h
  # ${imgui_SOURCES_DIR}/backends/imgui_impl_glfw.h
)

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
set_target_properties(travis_doc PROPERTIES FOLDER "external")

# stduuid
FetchContent_Declare(
    stduuid
    GIT_REPOSITORY https://github.com/mariusbancila/stduuid.git
    GIT_TAG v1.2.3
)
set(UUID_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(UUID_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(stduuid)

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

# tinygltf
FetchContent_Declare(
    tinygltf
    GIT_REPOSITORY https://github.com/syoyo/tinygltf.git
    GIT_TAG release
)
set(TINYGLTF_HEADER_ONLY ON CACHE BOOL "" FORCE)
set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF CACHE BOOL "" FORCE)
set(TINYGLTF_INSTALL OFF CACHE BOOL "" FORCE)
set(TINYGLTF_INSTALL_VENDOR OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(tinygltf)

# framework project
set(FRAMEWORK_PROJECT_NAME "MyosotisFW")

FILE(GLOB_RECURSE FRAMEWORK_SOURCES 
  "${PROJECT_SOURCE_DIR}/framework/include/*.h"
  "${PROJECT_SOURCE_DIR}/framework/src/*.cpp"
)
GroupSourcesByFolder(${FRAMEWORK_SOURCES})

add_library(${FRAMEWORK_PROJECT_NAME} ${FRAMEWORK_SOURCES} ${APP_INFO_RC})
AddIncludeDirectories(${FRAMEWORK_PROJECT_NAME} ${FRAMEWORK_SOURCES})

# build switch
target_compile_definitions(${FRAMEWORK_PROJECT_NAME} PRIVATE
  $<$<CONFIG:Debug>:DEBUG>
  $<$<CONFIG:FWDLL>:FWDLL>
  $<$<CONFIG:Release>:RELEASE>
)

# glfw
target_link_libraries(${FRAMEWORK_PROJECT_NAME} PRIVATE glfw)
# glm
target_link_libraries(${FRAMEWORK_PROJECT_NAME} PRIVATE glm::glm)
# ofbx
source_group("external/ofbx" FILES ${OFBX_SOURCES})
target_sources(${FRAMEWORK_PROJECT_NAME} PRIVATE ${OFBX_SOURCES})
target_include_directories(${FRAMEWORK_PROJECT_NAME} PRIVATE ${ofbx_SOURCES_DIR}/src)
# imgui
source_group("external/imgui" FILES ${IMGUI_SOURCES})
target_sources(${FRAMEWORK_PROJECT_NAME} PRIVATE ${IMGUI_SOURCES})
target_include_directories(${FRAMEWORK_PROJECT_NAME} PRIVATE ${imgui_SOURCES_DIR})
# vma
source_group("external/vma" FILES ${VMA_SOURCES})
target_sources(${FRAMEWORK_PROJECT_NAME} PRIVATE ${VMA_SOURCES})
target_include_directories(${FRAMEWORK_PROJECT_NAME} PRIVATE ${vma_SOURCE_DIR}/include/)
# RapidJSON
target_include_directories(${FRAMEWORK_PROJECT_NAME} PRIVATE ${RapidJSON_SOURCE_DIR}/include)
# stduuid
target_include_directories(${FRAMEWORK_PROJECT_NAME} PRIVATE ${stduuid_SOURCE_DIR})
target_include_directories(${FRAMEWORK_PROJECT_NAME} PRIVATE ${stduuid_SOURCE_DIR}/include)
# stb
source_group("external/stb" FILES ${STB_SOURCES})
target_sources(${FRAMEWORK_PROJECT_NAME} PRIVATE ${STB_SOURCES})
target_include_directories(${FRAMEWORK_PROJECT_NAME} PRIVATE ${stb_SOURCE_DIR})
# tinygltf
target_include_directories(${FRAMEWORK_PROJECT_NAME} PRIVATE ${tinygltf_SOURCE_DIR})
# vulkan
find_package(Vulkan REQUIRED)
target_include_directories(${FRAMEWORK_PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${FRAMEWORK_PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES})

# dll project
add_subdirectory(frameworkDLL)

# game project
add_subdirectory(myosotis)

# editor project
add_subdirectory(editor)