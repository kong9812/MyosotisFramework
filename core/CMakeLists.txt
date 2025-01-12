# Copyright (c) 2025 kong9812
cmake_minimum_required(VERSION 3.22)
include(FetchContent)

project(MyosotisFW)

set(CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

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

GroupSourcesByFolder(${PROJECT_NAME})

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)
FetchContent_MakeAvailable(glm)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)

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

# work directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
)

# build switch
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:RELEASE>
)
