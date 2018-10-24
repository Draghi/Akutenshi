cmake_minimum_required(VERSION 3.2) 

if(NOT DEFINED AK_BUILD_ENV_SETUP)
	include("cmake/SetupBuildEnvironment.cmake")
endif()

project(akrender VERSION 1.0.0.0)
sugar_include("src")
add_library(akrender STATIC ${AK_RENDER_SOURCE})

# ################### #
# # Compile Options # #
# ################### #
set_target_properties(akrender PROPERTIES CXX_EXTENSIONS OFF CXX_STANDARD_REQUIRED ON CXX_STANDARD 17)
target_compile_options(akrender PRIVATE ${AK_BUILD_CXX_WARNING_FLAGS})
target_compile_options(akrender PRIVATE ${AK_BUILD_CXX_COMPILER_FLAGS})

# ##################### #
# # Internal Includes # #
# ##################### #
target_include_directories(akrender PUBLIC  inc)
target_include_directories(akrender PRIVATE src)

target_include_directories(akrender SYSTEM PUBLIC ${AK_ASSET_INCLUDE_DIR})
target_include_directories(akrender SYSTEM PUBLIC ${AK_ENGINE_INCLUDE_DIR})
target_include_directories(akrender SYSTEM PUBLIC ${AK_INPUT_INCLUDE_DIR})

target_link_libraries(akrender PUBLIC akasset)
target_link_libraries(akrender PUBLIC akengine)
target_link_libraries(akrender PUBLIC akinput)

# ##################### #
# # External Includes # #
# ##################### #
target_include_directories(akrender SYSTEM PRIVATE ${GLFW_INCLUDE_DIR})
target_include_directories(akrender SYSTEM PRIVATE ${GL4_INCLUDE_DIR})

target_link_libraries(akrender PRIVATE glfw)
target_link_libraries(akrender PRIVATE GL4)