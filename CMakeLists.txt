cmake_minimum_required (VERSION 3.0.0)

project (EventEmitter)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/include)

set (_sources
	src/Emitter.cpp
	src/Registry.cpp
)

set (_headers
	include/Emitter.h
	include/Registry.h
	include/TypeTag.h
)

add_library (EventEmitter ${_sources} ${_headers})
set_target_properties(EventEmitter PROPERTIES DEBUG_POSTFIX d)


install (
	DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
	DESTINATION include/EventEmitter
)
install (
	TARGETS EventEmitter DESTINATION lib
)
set_property(TARGET EventEmitter PROPERTY CXX_STANDARD 11)