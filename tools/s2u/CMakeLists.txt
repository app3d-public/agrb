cmake_minimum_required(VERSION 3.17)

set(ARGS_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(ARGS_BUILD_UNITTESTS OFF CACHE BOOL "" FORCE)
if(NOT TARGET args)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../modules/3rd-party/args
        ${CMAKE_CURRENT_BINARY_DIR}/args-build EXCLUDE_FROM_ALL)
endif()

add_executable(agrb_s2u main.cpp)
target_link_libraries(agrb_s2u PRIVATE agrb args)
set_target_properties(agrb_s2u PROPERTIES
    CXX_EXTENSIONS YES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tools"
)
