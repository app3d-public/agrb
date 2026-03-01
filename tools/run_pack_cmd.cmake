if(NOT DEFINED S2U_EXE)
    message(FATAL_ERROR "S2U_EXE is required")
endif()
if(NOT DEFINED INPUT_DIR)
    message(FATAL_ERROR "INPUT_DIR is required")
endif()
if(NOT DEFINED OUTPUT_FILE)
    message(FATAL_ERROR "OUTPUT_FILE is required")
endif()
if(NOT DEFINED S2U_DIR)
    message(FATAL_ERROR "S2U_DIR is required")
endif()
if(NOT DEFINED AGRB_DIR)
    message(FATAL_ERROR "AGRB_DIR is required")
endif()

if(WIN32)
    set(_path_sep ";")
else()
    set(_path_sep ":")
endif()

set(_tool_path "${S2U_DIR}${_path_sep}${AGRB_DIR}${_path_sep}$ENV{PATH}")

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env "PATH=${_tool_path}"
            "${S2U_EXE}" -i "${INPUT_DIR}" -o "${OUTPUT_FILE}"
    RESULT_VARIABLE _pack_result
    OUTPUT_VARIABLE _pack_out
    ERROR_VARIABLE _pack_err
)

if(NOT _pack_result EQUAL 0)
    message(FATAL_ERROR "Shader pack failed for ${INPUT_DIR}\n${_pack_out}\n${_pack_err}")
endif()
