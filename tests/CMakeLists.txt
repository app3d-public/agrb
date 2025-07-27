cmake_minimum_required(VERSION 3.17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

set(TEST_LIBRARIES acul agrb)
set(TEST_DATA_DIR "${CMAKE_CURRENT_BINARY_DIR}/data")
set(TEST_OUTPUT_DIR "${CMAKE_BINARY_DIR}/tests/artifacts")
set(TEST_ENV "TEST_DATA_DIR=${TEST_DATA_DIR};TEST_OUTPUT_DIR=${TEST_OUTPUT_DIR}")
set(TEST_SOURCES "env.cpp")

# Prepare Shaders
function(add_shader INPUT_NAME OUTPUT_NAME)
    add_custom_command(
        OUTPUT ${OUTPUT_NAME}
        COMMAND glslc ${INPUT_NAME} -o ${OUTPUT_NAME}
        DEPENDS ${INPUT_NAME}
        COMMENT "Compiling shader ${INPUT_NAME} to SPIR-V binary ${OUTPUT_NAME}"
    )
endfunction()

add_shader(${CMAKE_CURRENT_SOURCE_DIR}/data/test.vert ${TEST_DATA_DIR}/vs_test.spv)
add_shader(${CMAKE_CURRENT_SOURCE_DIR}/data/test.frag ${TEST_DATA_DIR}/fs_test.spv)
add_custom_target(SHADERS ALL DEPENDS ${TEST_DATA_DIR}/vs_test.spv ${TEST_DATA_DIR}/fs_test.spv)

add_test_files(agrb buffer buffer.cpp)
add_test_files(agrb descriptors descriptors.cpp)
add_test_files(agrb utils utils.cpp)
add_test_files(agrb pipeline pipeline.cpp)
add_test_files(agrb vector vector.cpp)

if(ENABLE_COVERAGE)
    add_test_coverage(agrb)
endif()