add_library("logger" STATIC)

target_include_directories("logger" PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_sources("logger" PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/logger.cpp"
)

add_library("common::logger" ALIAS "logger")