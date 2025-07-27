cmake_minimum_required(VERSION 3.17)
project(agrb
    VERSION 0.0.1
    LANGUAGES CXX C
    DESCRIPTION "App3d Graphics Rendering Backend")

add_files(${PROJECT_NAME} RECURSE "${CMAKE_CURRENT_SOURCE_DIR}/src")
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/vk_mem_alloc.cpp PROPERTIES
    COMPILE_FLAGS "-w"
)
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
target_compile_definitions(${PROJECT_NAME} PRIVATE APPLIB_BUILD)
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/3rd-party/vma/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(${PROJECT_NAME} PRIVATE acul)

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

if(USE_ASAN)
    target_link_options(${PROJECT_NAME} PUBLIC -fsanitize=address -fno-omit-frame-pointer)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden)
target_compile_options(${PROJECT_NAME} PRIVATE -fvisibility-inlines-hidden)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS YES
)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()