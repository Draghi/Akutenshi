cmake_minimum_required(VERSION 3.2) 

if(NOT DEFINED AK_BUILD_ENV_SETUP)
	include("cmake/SetupBuildEnvironment.cmake")
endif()

project(aksound VERSION 1.0.0.0)
sugar_include("src")
add_library(aksound STATIC ${AK_SOUND_SOURCE})

# ################### #
# # Compile Options # #
# ################### #
set_target_properties(aksound PROPERTIES CXX_EXTENSIONS OFF CXX_STANDARD_REQUIRED ON CXX_STANDARD 17)
target_compile_options(aksound PRIVATE ${AK_BUILD_CXX_WARNING_FLAGS})
target_compile_options(aksound PRIVATE ${AK_BUILD_CXX_COMPILER_FLAGS})

# ##################### #
# # Internal Includes # #
# ##################### #
target_include_directories(aksound PUBLIC  inc)
target_include_directories(aksound PRIVATE src)

target_include_directories(aksound SYSTEM PUBLIC ${AK_ENGINE_INCLUDE_DIR})
target_link_libraries(aksound akengine)

# ##################### #
# # External Includes # #
# ##################### #
target_include_directories(aksound SYSTEM PUBLIC ${MINI_AL_INCLUDE_DIR})
