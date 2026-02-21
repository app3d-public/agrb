if(POLICY CMP0011)
    cmake_policy(SET CMP0011 NEW)
endif()

if(NOT DEFINED BUILD_DIR)
    message(FATAL_ERROR "BUILD_DIR is required")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/shader_tools.cmake")
run_shader_commands("${BUILD_DIR}")
