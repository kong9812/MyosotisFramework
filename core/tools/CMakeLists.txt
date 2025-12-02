# Copyright (c) 2025 kong9812

# meshoptimizer
FetchContent_Declare(
    meshoptimizer
    GIT_REPOSITORY https://github.com/zeux/meshoptimizer.git
    GIT_TAG        v0.25
)
set(MESHOPT_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(meshoptimizer)
set_target_properties(meshoptimizer PROPERTIES FOLDER "external")
FILE(GLOB meshoptimizer_SOURCES "${meshoptimizer_SOURCE_DIR}/src/*cpp") 

# MFModelConverter
add_subdirectory(MFModelConverter)