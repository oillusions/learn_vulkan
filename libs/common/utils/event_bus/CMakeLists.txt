add_library("event_bus" STATIC)

target_include_directories("event_bus" PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_sources("event_bus" PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/event_bus.cpp"
)

add_library("common::event_bus" ALIAS "event_bus")