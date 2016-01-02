cmake_minimum_required (VERSION 3.4.0)

project (EventEmitter)
if (NOT WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -v -std=c++14")
endif()
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

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
target_link_libraries(EventEmitter Threads::Threads)
set_property (TARGET EventEmitter PROPERTY CXX_STANDARD 14)
set_property (TARGET EventEmitter PROPERTY CXX_STANDARD_REQUIRED TRUE)

install (
	DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
	DESTINATION include/EventEmitter
)
install (
	TARGETS EventEmitter DESTINATION lib
)
option (ENABLE_TESTS "Enable Testing" OFF)
if (ENABLE_TESTS)
	enable_testing()
	add_subdirectory(test)
endif()
