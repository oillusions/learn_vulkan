add_library("resource" STATIC)

target_include_directories("resource" PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_sources("resource" PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/resource.cpp"
)

target_link_libraries("resource" PRIVATE
	lib::common::error
)

add_library("common::resource" ALIAS "resource")