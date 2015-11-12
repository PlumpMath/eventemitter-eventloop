set(_sources
	Application.cpp
	Backend.cpp
	EventEmitter.cpp
	EventLoop.cpp
)

set(_headers
	Application.h
	Backend.h
	EventEmitter.h
	EventLoop.h
	Log.h
	TypeTag.h

)

add_library(Core ${_sources} ${_headers})

target_include_directories (Core PUBLIC {CMAKE_CURRENT_SOURCE_DIR})