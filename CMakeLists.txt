cmake_minimum_required(VERSION 3.17)

if(NOT DEFINED ACBT_PROJECT_LOADED)
    include(cmake/project.cmake)
endif()

project(agrb
    VERSION 0.0.1
    LANGUAGES CXX C
    DESCRIPTION "App3d Graphics Rendering Backend")

if(NOT DEFINED ACBT_LOADED)
    include(cmake/utils.cmake)
endif()

gen_version_file(${CMAKE_CURRENT_BINARY_DIR}/include/${PROJECT_NAME}/version.h)

add_files(${PROJECT_NAME} RECURSE "${CMAKE_CURRENT_SOURCE_DIR}/src")
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/vk_mem_alloc.cpp PROPERTIES
    COMPILE_FLAGS "-w"
)
add_library(${PROJECT_NAME} SHARED ${AGRB_SRC})
target_compile_definitions(${PROJECT_NAME} PRIVATE APPLIB_BUILD)
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/3rd-party/vma/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(PROJECT_IS_TOP_LEVEL)
    add_subdirectory(modules/acul)
    add_subdirectory(modules/umbf)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC acul umbf)

if(WIN32)
    if(DEFINED ENV{VK_SDK_PATH})
        target_include_directories(${PROJECT_NAME} PUBLIC "$ENV{VK_SDK_PATH}/Include")
    else()
        message(FATAL_ERROR "VK_SDK_PATH is not set in the environment.")
    endif()

    if(NOT DEFINED VK_USE_PLATFORM_WIN32_KHR)
        target_compile_definitions(${PROJECT_NAME} PUBLIC VK_USE_PLATFORM_WIN32_KHR)
    endif()
elseif(UNIX AND NOT APPLE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC
        VK_USE_PLATFORM_XCB_KHR
        VK_USE_PLATFORM_WAYLAND_KHR
    )
elseif(APPLE AND NOT DEFINED VK_USE_PLATFORM_MACOS_MVK)
    target_compile_definitions(${PROJECT_NAME} PUBLIC VK_USE_PLATFORM_MACOS_MVK)
endif()

add_definitions(-DVK_NO_PROTOTYPES)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden)
target_compile_options(${PROJECT_NAME} PRIVATE -fvisibility-inlines-hidden)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    CXX_EXTENSIONS YES
)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if((NOT DEFINED BUILD_TOOLS OR BUILD_TOOLS) OR BUILD_TESTS)
    add_subdirectory(tools/s2u)
endif()

if(WIN32)
    gen_manifest_lib(lib${PROJECT_NAME} ${PROJECT_VERSION})
endif()
