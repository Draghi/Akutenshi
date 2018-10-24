cmake_minimum_required(VERSION 3.2) 
project(akrender VERSION 1.0.0.0)

if(NOT DEFINED AK_BUILD_ENV_SETUP)
	include("cmake/SetupBuildEnvironment.cmake")
endif()

sugar_include("src")
add_library(akrender STATIC ${AK_RENDER_SOURCE})

# ################### #
# # Compile Options # #
# ################### #
set_target_properties(akrender PROPERTIES CXX_EXTENSIONS OFF CXX_STANDARD_REQUIRED ON CXX_STANDARD 17)
target_compile_options(akrender PRIVATE ${AK_BUILD_CXX_WARNING_FLAGS})
target_compile_options(akrender PRIVATE ${AK_BUILD_CXX_COMPILER_FLAGS})

target_compile_definitions(akrender PUBLIC -DGLM_FORCE_RADIANS=1 -DGLM_ENABLE_EXPERIMENTAL=1 -DGLM_FORCE_LEFT_HANDED=1 -DGLM_FORCE_SIZE_FUNC=1 -DGLM_FORCE_INLINE=1)

# ##################### #
# # Internal Includes # #
# ##################### #
target_include_directories(akrender PUBLIC  inc)
target_include_directories(akrender PRIVATE src)

target_include_directories(akrender SYSTEM PUBLIC ${AK_ASSET_INCLUDE_DIR})
target_include_directories(akrender SYSTEM PUBLIC ${AK_COMMON_INCLUDE_DIR})
target_include_directories(akrender SYSTEM PUBLIC ${AK_ENGINE_INCLUDE_DIR})
target_include_directories(akrender SYSTEM PUBLIC ${AK_INPUT_INCLUDE_DIR})

target_link_libraries(akrender akasset)
target_link_libraries(akrender akcommon)
target_link_libraries(akrender akengine)
target_link_libraries(akrender akinput)

# ##################### #
# # External Includes # #
# ##################### #
target_include_directories(akrender SYSTEM PUBLIC ${GLFW_INCLUDE_DIR})
target_include_directories(akrender SYSTEM PUBLIC ${GL4_INCLUDE_DIR})

target_link_libraries(akrender glfw)
target_link_libraries(akrender GL4)