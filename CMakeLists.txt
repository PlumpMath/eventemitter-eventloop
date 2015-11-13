cmake_minimum_required (VERSION 3.0.0)

project (EventEmitter)

include_directories (${CMAKE_SOURCE_DIR}/include)

set (_sources
	src/EventEmitter.cpp
	src/EventLoop.cpp
)

set (_headers
	include/EventEmitter.h
	include/EventLoop.h
	include/TypeTag.h
)

add_library (EventEmitter ${_sources} ${_headers})

enable_testing ()
add_subdirectory (test)