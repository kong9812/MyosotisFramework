# Copyright (c) 2025 kong9812
set(EDITOR_PROJECT_NAME "MyosotisEngine")

FILE(GLOB_RECURSE EDITOR_SOURCES 
  "${PROJECT_SOURCE_DIR}/editor/include/*.h"
  "${PROJECT_SOURCE_DIR}/editor/src/*.cpp"
)
GroupSourcesByFolder(${EDITOR_SOURCES})

add_executable(${EDITOR_PROJECT_NAME} ${EDITOR_SOURCES})
target_link_libraries(${EDITOR_PROJECT_NAME} PRIVATE ${FRAMEWORK_PROJECT_NAME})

AddIncludeDirectories(${EDITOR_PROJECT_NAME} ${EDITOR_SOURCES})
AddIncludeDirectories(${EDITOR_PROJECT_NAME} ${FRAMEWORK_SOURCES})

# glfw
target_link_libraries(${EDITOR_PROJECT_NAME} PRIVATE glfw)
# glm
target_link_libraries(${EDITOR_PROJECT_NAME} PRIVATE glm::glm)
# ofbx
source_group("external/ofbx" FILES ${OFBX_SOURCES})
target_sources(${EDITOR_PROJECT_NAME} PRIVATE ${OFBX_SOURCES})
target_include_directories(${EDITOR_PROJECT_NAME} PRIVATE ${ofbx_SOURCES_DIR}/src)
# imgui
source_group("external/imgui" FILES ${IMGUI_SOURCES})
target_sources(${EDITOR_PROJECT_NAME} PRIVATE ${IMGUI_SOURCES})
target_include_directories(${EDITOR_PROJECT_NAME} PRIVATE ${imgui_SOURCES_DIR})
# vma
source_group("external/vma" FILES ${VMA_SOURCES})
target_sources(${EDITOR_PROJECT_NAME} PRIVATE ${VMA_SOURCES})
target_include_directories(${EDITOR_PROJECT_NAME} PRIVATE ${vma_SOURCE_DIR}/include/)
# RapidJSON
target_include_directories(${EDITOR_PROJECT_NAME} PRIVATE ${RapidJSON_SOURCE_DIR}/include)
# stduuid
target_include_directories(${EDITOR_PROJECT_NAME} PRIVATE ${stduuid_SOURCE_DIR})
target_include_directories(${EDITOR_PROJECT_NAME} PRIVATE ${stduuid_SOURCE_DIR}/include)
# stb
source_group("external/stb" FILES ${STB_SOURCES})
target_sources(${EDITOR_PROJECT_NAME} PRIVATE ${STB_SOURCES})
target_include_directories(${EDITOR_PROJECT_NAME} PRIVATE ${stb_SOURCE_DIR})
# vulkan
find_package(Vulkan REQUIRED)
target_include_directories(${EDITOR_PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${EDITOR_PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES})

# work directory
set_target_properties(${EDITOR_PROJECT_NAME} PROPERTIES
  VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
)

# build switch
target_compile_definitions(${EDITOR_PROJECT_NAME} PRIVATE
  $<$<CONFIG:Debug>:DEBUG>
  $<$<CONFIG:FWDLL>:FWDLL>
  $<$<CONFIG:Release>:RELEASE>
)

# FWDLL構成にデバッグ用のフラグを設定
target_compile_options(${EDITOR_PROJECT_NAME} PRIVATE 
  $<$<CONFIG:FWDLL>:/Zi>  # Debug構成に /Zi を指定
)
target_link_options(${EDITOR_PROJECT_NAME} PRIVATE 
  $<$<CONFIG:FWDLL>:/DEBUG>
)

# QT
target_compile_options(${EDITOR_PROJECT_NAME} PRIVATE 
  /Zc:__cplusplus /permissive-
)
set(Qt6_DIR "${PROJECT_SOURCE_DIR}/../qt/qtbase/lib/cmake/Qt6")
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
target_link_libraries(${EDITOR_PROJECT_NAME}
    PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)
target_include_directories(${EDITOR_PROJECT_NAME}
    PRIVATE
    "${PROJECT_SOURCE_DIR}/../qt/qtbase/include"
)
add_custom_command(TARGET ${EDITOR_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Qt6::Core>
        $<TARGET_FILE:Qt6::Gui>
        $<TARGET_FILE:Qt6::Widgets>
        $<TARGET_FILE_DIR:${EDITOR_PROJECT_NAME}>
)
add_custom_command(TARGET ${EDITOR_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:${EDITOR_PROJECT_NAME}>/platforms

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/../qt/qtbase/plugins/platforms/qwindows.dll"
        "${PROJECT_SOURCE_DIR}/../qt/qtbase/plugins/platforms/qwindowsd.dll"
        $<TARGET_FILE_DIR:${EDITOR_PROJECT_NAME}>/platforms
)