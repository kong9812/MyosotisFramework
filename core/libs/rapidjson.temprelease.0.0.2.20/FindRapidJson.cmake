cmake_minimum_required(VERSION 3.9)

if (NOT RapidJson_FOUND)
    add_library(RapidJson INTERFACE IMPORTED GLOBAL)
    set_target_properties(RapidJson PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/build/native/include")
endif()