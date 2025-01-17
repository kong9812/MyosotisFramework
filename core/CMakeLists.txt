# Copyright (c) 2025 kong9812
cmake_minimum_required(VERSION 3.22)
include(FetchContent)

project(MyosotisFW)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(DLL_PROJECT_NAME "myosotis")
set(CMAKE_CONFIGURATION_TYPES "Debug;FWDLL;Release" CACHE STRING "" FORCE)

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

add_executable(${PROJECT_NAME} ${SOURCES})

# ライブラリの作成
add_library(${DLL_PROJECT_NAME} SHARED ${SOURCES})
# set_target_properties(${DLL_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

GroupSourcesByFolder(${PROJECT_NAME})
GroupSourcesByFolder(${DLL_PROJECT_NAME})

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
target_link_libraries(${DLL_PROJECT_NAME} PRIVATE glfw)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)
FetchContent_MakeAvailable(glm)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)
target_link_libraries(${DLL_PROJECT_NAME} PRIVATE glm::glm)

# ofbx
FetchContent_Declare(
    ofbx
    GIT_REPOSITORY https://github.com/nem0/OpenFBX
    GIT_TAG master
)
FetchContent_MakeAvailable(ofbx)

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
