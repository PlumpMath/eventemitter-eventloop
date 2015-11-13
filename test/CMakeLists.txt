include_directories (${EventEmitter_INCLUDE_DIR})

add_executable(GenericTest generic_test.cpp)
target_link_libraries(GenericTest EventEmitter)

add_test (GenericTest GenericTest)