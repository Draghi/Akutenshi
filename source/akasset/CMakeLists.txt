cmake_minimum_required(VERSION 3.2) 
project(akasset VERSION 1.0.0.0)

if(NOT DEFINED AK_BUILD_ENV_SETUP)
	include("cmake/SetupBuildEnvironment.cmake")
endif()

sugar_include("src")

add_library(akasset STATIC ${AK_ASSET_SOURCE})

# ##################### #
# # Internal Includes # #
# ##################### #
target_include_directories(akasset PUBLIC  inc)
target_include_directories(akasset PRIVATE src)

target_include_directories(akasset SYSTEM PUBLIC ${AK_COMMON_INCLUDE_DIR})
target_include_directories(akasset SYSTEM PUBLIC ${AK_ENGINE_INCLUDE_DIR})
target_include_directories(akasset SYSTEM PUBLIC ${AK_RENDER_INCLUDE_DIR})
target_include_directories(akasset SYSTEM PUBLIC ${AK_MATH_INCLUDE_DIR})

target_link_libraries(akasset PUBLIC akcommon)
target_link_libraries(akasset PUBLIC akengine)
target_link_libraries(akasset PUBLIC akrender)
target_link_libraries(akasset PUBLIC akmath)

# ##################### #
# # External Includes # #
# ##################### #
target_include_directories(akasset SYSTEM PUBLIC ${RAPIDJSON_INCLUDE_DIR})
target_include_directories(akasset SYSTEM PUBLIC ${STB_INCLUDE_DIR})

# ################### #
# # Compile Options # #
# ################### #
target_compile_options(akasset PRIVATE ${AK_BUILD_CXX_WARNING_FLAGS})
target_compile_options(akasset PRIVATE ${AK_BUILD_CXX_COMPILER_FLAGS})
set_target_properties(akasset PROPERTIES CXX_EXTENSIONS OFF CXX_STANDARD_REQUIRED ON CXX_STANDARD 17)

# ############# #
# # Libraries # #
# ############# #

