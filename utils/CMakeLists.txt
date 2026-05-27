add_library("utils" STATIC)

target_include_directories("utils" PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

file(GLOB_RECURSE source_files CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
target_sources("utils" PRIVATE
    "${source_files}"
)

target_link_libraries("utils" PUBLIC
    Vulkan::Vulkan
    
    common::error
    common::logger
)

add_library("utils::utils" ALIAS "utils")

