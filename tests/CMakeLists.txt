cmake_minimum_required(VERSION 3.17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

set(TEST_LIBRARIES acul agrb)
set(TEST_DATA_DIR "${CMAKE_CURRENT_BINARY_DIR}/data")
set(TEST_OUTPUT_DIR "${CMAKE_BINARY_DIR}/tests/artifacts")
set(TEST_ENV "TEST_DATA_DIR=${TEST_DATA_DIR};TEST_OUTPUT_DIR=${TEST_OUTPUT_DIR}")
set(TEST_SOURCES "env.cpp")

file(MAKE_DIRECTORY ${TEST_DATA_DIR})
file(GLOB TEST_SHADER_STALE_FILES
    "${TEST_DATA_DIR}/*.spv"
    "${TEST_DATA_DIR}/*.spv.cmd"
    "${TEST_DATA_DIR}/*.d"
    "${TEST_DATA_DIR}/*.umlib"
)
if(TEST_SHADER_STALE_FILES)
    file(REMOVE ${TEST_SHADER_STALE_FILES})
endif()
find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(TEST_SHADER_MANIFEST "${CMAKE_CURRENT_SOURCE_DIR}/data/shaders.yaml")
set(TEST_SHADER_ENV "${CMAKE_CURRENT_SOURCE_DIR}/data/env.yaml")
set(TEST_SHADER_NAMESPACE "test")
set(TEST_SHADER_NAMESPACE_DIR "${TEST_DATA_DIR}/${TEST_SHADER_NAMESPACE}")
set(TEST_SHADER_DEPFILE "${TEST_SHADER_NAMESPACE_DIR}/agrb_shaders.d")
set(TEST_SHADER_UMLIB "${TEST_DATA_DIR}/test_shaders.umlib")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(TEST_SHADER_PROFILE "debug")
else()
    set(TEST_SHADER_PROFILE "release")
endif()

execute_process(
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../tools/shader_builder.py
        --env ${TEST_SHADER_ENV}
        -i ${TEST_SHADER_MANIFEST}
        -b ${TEST_DATA_DIR}
        --namespace ${TEST_SHADER_NAMESPACE}
        --profile ${TEST_SHADER_PROFILE}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE TEST_SHADER_GEN_RESULT
)
if(NOT TEST_SHADER_GEN_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to generate test shader command files")
endif()

file(GLOB TEST_SHADER_CMD_FILES "${TEST_SHADER_NAMESPACE_DIR}/*.spv.cmd")
if(NOT TEST_SHADER_CMD_FILES)
    message(FATAL_ERROR "No shader .cmd files were generated in ${TEST_DATA_DIR}")
endif()

set(TEST_SHADER_SPV_FILES)
foreach(TEST_SHADER_CMD_FILE IN LISTS TEST_SHADER_CMD_FILES)
    string(REGEX REPLACE "\\.cmd$" "" TEST_SHADER_SPV_FILE "${TEST_SHADER_CMD_FILE}")
    add_custom_command(
        OUTPUT ${TEST_SHADER_SPV_FILE}
        COMMAND cmd /c "${TEST_SHADER_CMD_FILE}"
        DEPENDS ${TEST_SHADER_CMD_FILE} ${TEST_SHADER_DEPFILE}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Compiling shader ${TEST_SHADER_SPV_FILE}"
        VERBATIM
    )
    list(APPEND TEST_SHADER_SPV_FILES ${TEST_SHADER_SPV_FILE})
endforeach()

add_custom_command(
    OUTPUT ${TEST_SHADER_UMLIB}
    COMMAND "${CMAKE_COMMAND}" -E env
            "PATH=$<TARGET_FILE_DIR:agrb_s2u>;$<TARGET_FILE_DIR:agrb>;$ENV{PATH}"
            "$<TARGET_FILE:agrb_s2u>"
            --input ${TEST_SHADER_NAMESPACE_DIR}
            --output ${TEST_SHADER_UMLIB}
    DEPENDS ${TEST_SHADER_SPV_FILES} agrb_s2u
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Packing shaders into ${TEST_SHADER_UMLIB}"
    VERBATIM
)

add_custom_target(SHADERS ALL DEPENDS ${TEST_SHADER_UMLIB})

add_test_files(agrb buffer buffer.cpp)
add_test_files(agrb descriptors descriptors.cpp)
add_test_files(agrb utils utils.cpp)
add_test_files(agrb pipeline pipeline.cpp)
add_test_files(agrb vector vector.cpp)
add_dependencies(agrb_pipeline SHADERS)

if(ENABLE_COVERAGE)
    add_test_coverage(agrb)
endif()
