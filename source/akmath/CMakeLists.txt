cmake_minimum_required(VERSION 3.2) 
project(akmath VERSION 1.0.0.0)

if(NOT DEFINED AK_BUILD_ENV_SETUP)
	include("cmake/SetupBuildEnvironment.cmake")
endif()

sugar_include("src")
add_library(akmath STATIC ${AK_MATH_SOURCE})

# ################### #
# # Compile Options # #
# ################### #
set_target_properties(akmath PROPERTIES CXX_EXTENSIONS OFF CXX_STANDARD_REQUIRED ON CXX_STANDARD 17)
target_compile_options(akmath PRIVATE ${AK_BUILD_CXX_WARNING_FLAGS})
target_compile_options(akmath PRIVATE ${AK_BUILD_CXX_COMPILER_FLAGS})
target_compile_definitions(akmath PUBLIC -DGLM_FORCE_RADIANS=1 -DGLM_ENABLE_EXPERIMENTAL=1 -DGLM_FORCE_LEFT_HANDED=1 -DGLM_FORCE_SIZE_FUNC=1 -DGLM_FORCE_INLINE=1)

# ##################### #
# # Internal Includes # #
# ##################### #
target_include_directories(akmath PUBLIC  inc)
target_include_directories(akmath PRIVATE src)

target_include_directories(akmath SYSTEM PUBLIC ${AK_COMMON_INCLUDE_DIR})
target_include_directories(akmath SYSTEM PUBLIC ${AK_ENGINE_INCLUDE_DIR})

target_link_libraries(akmath PUBLIC akcommon)
target_link_libraries(akmath PUBLIC akengine)

# ##################### #
# # External Includes # #
# ##################### #
target_include_directories(akmath SYSTEM PUBLIC ${GLM_INCLUDE_DIR})
