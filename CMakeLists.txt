cmake_minimum_required(VERSION 3.22)

project(MyosotisFW)

set(CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE SOURCES 
  "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"

  "${CMAKE_CURRENT_SOURCE_DIR}/libs/OpenFBX/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/libs/OpenFBX/*.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/libs/OpenFBX/*.cpp"
)

# Ref: https://github.com/TheLartians/GroupSourcesByFolder.cmake
# Thank you :)
function(GroupSourcesByFolder target)
  set(SOURCE_GROUP_DELIMITER "/")
  set(last_dir "")
  set(files "")

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
  endforeach()

  if(files)
    source_group("${last_dir}" FILES ${files})
  endif()
endfunction()

# if (WIN32) 
#     add_executable(MyosotisFW WIN32 ${SOURCES})
# elseif(UNIX)
#     add_executable(MyosotisFW ${SOURCES})
# endif()
add_executable(MyosotisFW ${SOURCES})

GroupSourcesByFolder(MyosotisFW)

target_include_directories(MyosotisFW PRIVATE ./include)
target_include_directories(MyosotisFW PRIVATE ./include/asset)

# GLFW
target_include_directories(MyosotisFW PRIVATE ./libs/glfw.3.4.0/include)
link_directories(./libs/glfw.3.4.0/lib-mingw-w64)
target_link_libraries(MyosotisFW PRIVATE ${PROJECT_SOURCE_DIR}/libs/glfw.3.4.0/lib-vc2022/glfw3.lib)
# GLM
target_include_directories(MyosotisFW PRIVATE ./libs/glm.1.0.1/build/native/include)
# GITF
target_include_directories(MyosotisFW PRIVATE ./libs/Microsoft.glTF.CPP.1.6.3.1/build/native/include)
link_directories(./libs/Microsoft.glTF.CPP.1.6.3.1/build/native/lib/x64/Debug)
target_link_libraries(MyosotisFW PRIVATE ${PROJECT_SOURCE_DIR}/libs/Microsoft.glTF.CPP.1.6.3.1/build/native/lib/x64/Debug/GLTFSDK.lib)
# repidjson
target_include_directories(MyosotisFW PRIVATE ./libs/rapidjson.temprelease.0.0.2.20/build/native/include)
# ofbx
target_include_directories(MyosotisFW PRIVATE ./libs/OpenFBX)

# Vulkan
find_package(Vulkan REQUIRED)
target_include_directories(MyosotisFW PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(MyosotisFW PRIVATE ${Vulkan_LIBRARIES})

# work directory
set_target_properties(MyosotisFW PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

# build switch
target_compile_definitions(MyosotisFW PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:RELEASE>
)

