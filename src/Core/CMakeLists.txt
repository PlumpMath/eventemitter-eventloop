set(_sources
	Application.cpp
)

set(_headers
	Application.h	
)

add_library(Core ${_sources} ${_headers})

target_include_directories (Core PUBLIC {CMAKE_CURRENT_SOURCE_DIR})