cmake_minimum_required(VERSION 3.12)

project("learn" VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

add_compile_definitions(VULKAN_HPP_NO_EXCEPTIONS)

add_executable(${CMAKE_PROJECT_NAME} "${PROJECT_SOURCE_DIR}/project/main.cpp")


target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
    "${CMAKE_SOURCE_DIR}/project/include"
)

file(GLOB_RECURSE source_files CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/project/src/*.cpp")
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${source_files}
)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(stb_image REQUIRED)

add_subdirectory(libs/common/utils/logger)
add_subdirectory(libs/common/utils/error)
add_subdirectory(libs/common/utils/event_bus)

add_subdirectory(utils)
add_subdirectory(platform)
add_subdirectory(vulkan)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    Vulkan::Vulkan
    glfw

    common::logger
    common::error
    common::event_bus

    utils::utils

    platform::platform
    vulkan::vulkan_core
)

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
	target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
		-static-libgcc
		-static-libstdc++
		-static
	)
endif()

