add_library("vulkan" STATIC)

target_include_directories("vulkan" PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

file(GLOB_RECURSE source_files CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
target_sources("vulkan" PRIVATE
    "${source_files}"
)

target_link_libraries("vulkan" PRIVATE
    Vulkan::Vulkan
    common::error

    utils::utils
)

add_library("vulkan::vulkan_core" ALIAS "vulkan")

