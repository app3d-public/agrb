find_package(Python3 REQUIRED COMPONENTS Interpreter)
set(AGRB_SHADER_TOOLS_DIR "${CMAKE_CURRENT_LIST_DIR}")

if(POLICY CMP0116)
    cmake_policy(SET CMP0116 NEW)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(SHADERS_PROFILE "debug")
else()
    set(SHADERS_PROFILE "release")
endif()

function(get_shader_namespaces SHADERS_MANIFEST OUT_VAR)
    set(_ns_script "${AGRB_SHADER_TOOLS_DIR}/get_shaders_ns.py")

    if(NOT EXISTS "${_ns_script}")
        message(FATAL_ERROR "Shader namespace script not found: ${_ns_script}")
    endif()

    execute_process(
        COMMAND "${Python3_EXECUTABLE}" "${_ns_script}" -i "${SHADERS_MANIFEST}"
        OUTPUT_VARIABLE _ns_output
        ERROR_VARIABLE _ns_error
        RESULT_VARIABLE _ns_status
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(NOT _ns_status EQUAL 0)
        message(FATAL_ERROR "Failed to get shader namespaces from ${SHADERS_MANIFEST}: ${_ns_error}")
    endif()

    if(_ns_output STREQUAL "")
        set(${OUT_VAR} "" PARENT_SCOPE)
        return()
    endif()

    string(REPLACE "\r\n" "\n" _ns_output "${_ns_output}")
    string(REPLACE "\n" ";" _ns_list "${_ns_output}")
    set(${OUT_VAR} ${_ns_list} PARENT_SCOPE)
endfunction()

function(run_shader_commands BUILD_DIR)
    file(GLOB CMD_FILES "${BUILD_DIR}/*.spv.cmd")

    if(NOT CMD_FILES)
        message(FATAL_ERROR "No shader command files found in: ${BUILD_DIR}")
    endif()

    foreach(CMD_FILE IN LISTS CMD_FILES)
        if(WIN32)
            execute_process(
                COMMAND cmd /C call "${CMD_FILE}"
                RESULT_VARIABLE CMD_RESULT
            )
        else()
            execute_process(
                COMMAND sh "${CMD_FILE}"
                RESULT_VARIABLE CMD_RESULT
            )
        endif()

        if(NOT CMD_RESULT EQUAL 0)
            message(FATAL_ERROR "Shader compile command failed: ${CMD_FILE}")
        endif()
    endforeach()
endfunction()

function(add_shaders ENV_MANIFEST SHADERS_MANIFEST OUT_DIR)
    set(options)
    set(oneValueArgs TARGET_PREFIX BUILD_DIR WORKING_DIRECTORY S2U_TARGET PROFILE)
    cmake_parse_arguments(ADD_SHADERS "${options}" "${oneValueArgs}" "" ${ARGN})

    if(NOT EXISTS "${ENV_MANIFEST}")
        message(FATAL_ERROR "Shader env manifest not found: ${ENV_MANIFEST}")
    endif()
    if(NOT EXISTS "${SHADERS_MANIFEST}")
        message(FATAL_ERROR "Shader manifest not found: ${SHADERS_MANIFEST}")
    endif()

    if(NOT ADD_SHADERS_TARGET_PREFIX)
        set(ADD_SHADERS_TARGET_PREFIX "SHADERS")
    endif()

    if(NOT ADD_SHADERS_BUILD_DIR)
        string(TOLOWER "${ADD_SHADERS_TARGET_PREFIX}" _shader_prefix_lower)
        set(ADD_SHADERS_BUILD_DIR "${CMAKE_BINARY_DIR}/shaders/${_shader_prefix_lower}")
    endif()

    if(NOT ADD_SHADERS_WORKING_DIRECTORY)
        set(ADD_SHADERS_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
    endif()

    if(NOT ADD_SHADERS_S2U_TARGET)
        set(ADD_SHADERS_S2U_TARGET "agrb_s2u")
    endif()

    if(NOT ADD_SHADERS_PROFILE)
        set(ADD_SHADERS_PROFILE "${SHADERS_PROFILE}")
    endif()

    set(SHADERS_ENV_FILE "${ENV_MANIFEST}")
    set(SHADERS_BUILDER "${AGRB_SHADER_TOOLS_DIR}/shader_builder.py")
    set(SHADERS_RUN_CMDS "${AGRB_SHADER_TOOLS_DIR}/run_shader_cmds.cmake")
    set(SHADERS_OUT_ROOT "${OUT_DIR}")
    set(SHADERS_BUILD_ROOT "${ADD_SHADERS_BUILD_DIR}")
    set(SHADERS_GEN_DEPFILE "${SHADERS_BUILD_ROOT}/agrb_shaders_all.d")

    get_shader_namespaces("${SHADERS_MANIFEST}" SHADER_NAMESPACES)
    if(NOT SHADER_NAMESPACES)
        message(FATAL_ERROR "No shader namespaces found in ${SHADERS_MANIFEST}")
    endif()

    set(SHADERS_GROUP_HEADERS)
    set(SHADERS_GROUP_DEPFILES)
    set(SHADERS_GROUP_LIBS)

    foreach(NS IN LISTS SHADER_NAMESPACES)
        set(GROUP_BUILD_DIR "${SHADERS_BUILD_ROOT}/${NS}")
        set(GROUP_HEADER "${GROUP_BUILD_DIR}/shaders.h")
        set(GROUP_DEPFILE "${GROUP_BUILD_DIR}/agrb_shaders.d")
        set(GROUP_LIBRARY "${SHADERS_OUT_ROOT}/${NS}.umlib")
        list(APPEND SHADERS_GROUP_HEADERS "${GROUP_HEADER}")
        list(APPEND SHADERS_GROUP_DEPFILES "${GROUP_DEPFILE}")
        list(APPEND SHADERS_GROUP_LIBS "${GROUP_LIBRARY}")
    endforeach()

    set(SHADERS_GEN_STAMP "${SHADERS_BUILD_ROOT}/.gen.stamp")
    add_custom_command(
        OUTPUT "${SHADERS_GEN_STAMP}"
        BYPRODUCTS ${SHADERS_GROUP_HEADERS} ${SHADERS_GROUP_DEPFILES}
        COMMAND "${CMAKE_COMMAND}" -E rm -rf "${SHADERS_BUILD_ROOT}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${SHADERS_BUILD_ROOT}"
        COMMAND "${Python3_EXECUTABLE}" "${SHADERS_BUILDER}"
                --env "${SHADERS_ENV_FILE}"
                -i "${SHADERS_MANIFEST}"
                -b "${SHADERS_BUILD_ROOT}"
                --profile "${ADD_SHADERS_PROFILE}"
                --aggregate-target "${SHADERS_GEN_STAMP}"
        COMMAND "${CMAKE_COMMAND}" -E touch "${SHADERS_GEN_STAMP}"
        DEPENDS "${SHADERS_BUILDER}" "${SHADERS_ENV_FILE}" "${SHADERS_MANIFEST}"
        DEPFILE "${SHADERS_GEN_DEPFILE}"
        WORKING_DIRECTORY "${ADD_SHADERS_WORKING_DIRECTORY}"
        COMMENT "Generating shader commands (${ADD_SHADERS_TARGET_PREFIX})"
        VERBATIM
    )

    set(SHADERS_GEN_TARGET "${ADD_SHADERS_TARGET_PREFIX}_GEN")
    add_custom_target(${SHADERS_GEN_TARGET} DEPENDS "${SHADERS_GEN_STAMP}")

    foreach(NS IN LISTS SHADER_NAMESPACES)
        set(GROUP_BUILD_DIR "${SHADERS_BUILD_ROOT}/${NS}")
        set(GROUP_HEADER "${GROUP_BUILD_DIR}/shaders.h")
        set(GROUP_DEPFILE "${GROUP_BUILD_DIR}/agrb_shaders.d")
        set(GROUP_STAMP "${GROUP_BUILD_DIR}/.spv.stamp")
        set(GROUP_LIBRARY "${SHADERS_OUT_ROOT}/${NS}.umlib")

        add_custom_command(
            OUTPUT "${GROUP_STAMP}"
            COMMAND "${CMAKE_COMMAND}" -DBUILD_DIR=${GROUP_BUILD_DIR} -P "${SHADERS_RUN_CMDS}"
            COMMAND "${CMAKE_COMMAND}" -E touch "${GROUP_STAMP}"
            DEPENDS ${SHADERS_GEN_TARGET} "${GROUP_HEADER}" "${GROUP_DEPFILE}"
            WORKING_DIRECTORY "${ADD_SHADERS_WORKING_DIRECTORY}"
            COMMENT "Compiling shaders for ${NS} (${ADD_SHADERS_TARGET_PREFIX})"
            VERBATIM
        )

        add_custom_command(
            OUTPUT "${GROUP_LIBRARY}"
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${SHADERS_OUT_ROOT}"
            COMMAND "${CMAKE_COMMAND}" -E env
                    "PATH=$<TARGET_FILE_DIR:${ADD_SHADERS_S2U_TARGET}>;$<TARGET_FILE_DIR:agrb>;$ENV{PATH}"
                    "$<TARGET_FILE:${ADD_SHADERS_S2U_TARGET}>"
                    -i "${GROUP_BUILD_DIR}"
                    -o "${GROUP_LIBRARY}"
            DEPENDS "${GROUP_STAMP}" ${ADD_SHADERS_S2U_TARGET}
            COMMENT "Packing ${NS} shaders (${ADD_SHADERS_TARGET_PREFIX})"
            VERBATIM
        )
    endforeach()

    add_custom_target(${ADD_SHADERS_TARGET_PREFIX} DEPENDS ${SHADERS_GROUP_LIBS} ${SHADERS_GROUP_HEADERS})
endfunction()
