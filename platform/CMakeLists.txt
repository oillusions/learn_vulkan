add_library("platform" STATIC)

target_include_directories("platform" PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

file(GLOB_RECURSE source_files CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

target_sources("platform" PRIVATE
    ${source_files}
)

target_link_libraries("platform" PRIVATE
    Vulkan::Vulkan
    glfw

    common::error
    common::event_bus
)

add_library("platform::platform" ALIAS "platform")