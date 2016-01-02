link_directories (${CMAKE_CURRENT_BINARY_DIR}/external/lib/)
include_directories (${EventEmitter_INCLUDE_DIR})
include(ExternalProject)
add_executable(UnitTests generic_test.cpp)

ExternalProject_Add (googletest-external
	PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external-build
	SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/googletest
	INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/external
	CMAKE_ARGS
		-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
	STEP_TARGETS post_install
)


target_include_directories (UnitTests PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/external/include)

add_dependencies(UnitTests googletest-external)
target_link_libraries(UnitTests EventEmitter gtest)

add_test (UnitTests UnitTests)