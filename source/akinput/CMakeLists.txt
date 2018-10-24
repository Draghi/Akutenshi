cmake_minimum_required(VERSION 3.2) 

if(NOT DEFINED AK_BUILD_ENV_SETUP)
	include("cmake/SetupBuildEnvironment.cmake")
endif()

project(akinput VERSION 1.0.0.0)
sugar_include("src")
add_library(akinput STATIC ${AK_INPUT_SOURCE})

# ################### #
# # Compile Options # #
# ################### #
set_target_properties(akinput PROPERTIES CXX_EXTENSIONS OFF CXX_STANDARD_REQUIRED ON CXX_STANDARD 17)
target_compile_options(akinput PRIVATE ${AK_BUILD_CXX_WARNING_FLAGS})
target_compile_options(akinput PRIVATE ${AK_BUILD_CXX_COMPILER_FLAGS})

# ##################### #
# # Internal Includes # #
# ##################### #
target_include_directories(akinput PUBLIC  inc)
target_include_directories(akinput PRIVATE src)

target_include_directories(akinput SYSTEM PUBLIC ${AK_ENGINE_INCLUDE_DIR})
target_link_libraries(akinput akengine)

# ##################### #
# # External Includes # #
# ##################### #

