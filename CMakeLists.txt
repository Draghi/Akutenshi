cmake_minimum_required(VERSION 3.2) 

if(NOT DEFINED AK_BUILD_ENV_SETUP)
	include("cmake/SetupBuildEnvironment.cmake")
endif()

add_subdirectory("${CMAKE_SOURCE_DIR}/source/akengine")

add_subdirectory("${CMAKE_SOURCE_DIR}/source/akasset")

add_subdirectory("${CMAKE_SOURCE_DIR}/source/akrender")
add_subdirectory("${CMAKE_SOURCE_DIR}/source/akinput")
add_subdirectory("${CMAKE_SOURCE_DIR}/source/aksound")

add_subdirectory("${CMAKE_SOURCE_DIR}/source/akgame")

add_subdirectory("${CMAKE_SOURCE_DIR}/source/akmain")