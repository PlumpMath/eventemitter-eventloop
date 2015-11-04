cmake_minimum_required (VERSION 3.0.0)

project (simpleSprites)

set (PROJECT_ROOT_DIR "${CMAKE_SOURCE_DIR}/../")
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_ROOT_DIR}/cmake/Modules/")
include_directories (${CMAKE_SOURCE_DIR})

find_package (GSL REQUIRED)
find_package (SFML COMPONENTS graphics window system REQUIRED)
find_package (UNITTEST++ REQUIRED)
add_subdirectory (Core)


set(_sources
    main.cpp
)
add_executable (SimpleSprites ${_sources})
target_link_libraries (SimpleSprites LINK_PUBLIC Core ${SFML_LIBRARIES})

add_executable (TestApp test.cpp)
target_link_libraries (TestApp LINK_PUBLIC Core ${SFML_LIBRARIES})