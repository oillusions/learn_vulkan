add_library("error" STATIC)

target_include_directories("error" PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_sources("error" PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/error.cpp"
)

add_library("common::error" ALIAS "error")