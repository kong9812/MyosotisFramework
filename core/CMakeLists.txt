# Copyright (c) 2025 kong9812
cmake_minimum_required(VERSION 3.22)

project(MyosotisFW)

set(CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE SOURCES 
  "${PROJECT_SOURCE_DIR}/include/*.h"
  "${PROJECT_SOURCE_DIR}/src/*.cpp"

  "${PROJECT_SOURCE_DIR}/libs/OpenFBX/*.h"
  "${PROJECT_SOURCE_DIR}/libs/OpenFBX/*.c"
  "${PROJECT_SOURCE_DIR}/libs/OpenFBX/*.cpp"
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

# if (WIN32) 
#     add_executable(${PROJECT_NAME} WIN32 ${SOURCES})
# elseif(UNIX)
#     add_executable(${PROJECT_NAME} ${SOURCES})
# endif()
add_executable(${PROJECT_NAME} ${SOURCES})

GroupSourcesByFolder(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include/asset)

# GLFW
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/libs/glfw.3.4.0/include)
link_directories(${PROJECT_SOURCE_DIR}/libs/glfw.3.4.0/lib-mingw-w64)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/libs/glfw.3.4.0/lib-vc2022/glfw3.lib)
# GLM
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/libs/glm.1.0.1/build/native/include)
# GITF
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/libs/Microsoft.glTF.CPP.1.6.3.1/build/native/include)
link_directories(${PROJECT_SOURCE_DIR}/libs/Microsoft.glTF.CPP.1.6.3.1/build/native/lib/x64/Debug)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/libs/Microsoft.glTF.CPP.1.6.3.1/build/native/lib/x64/Debug/GLTFSDK.lib)
# repidjson
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/libs/rapidjson.temprelease.0.0.2.20/build/native/include)
# ofbx
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/libs/OpenFBX)

# Vulkan
find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES})

# work directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
)

# build switch
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:RELEASE>
)

