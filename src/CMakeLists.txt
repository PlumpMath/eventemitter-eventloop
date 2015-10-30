cmake_minimum_required (VERSION 3.0.0)

project(simpleSprites)

add_subdirectory (Core)

set(_sources
	main.cpp
)
add_executable(app ${_sources})
target_link_libraries (app LINK_PUBLIC Core)