cmake_minimum_required(VERSION 3.2) 

if(NOT DEFINED AK_BUILD_ENV_SETUP)
	include("cmake/SetupBuildEnvironment.cmake")
endif()

project(akasset VERSION 1.0.0.0)
sugar_include("src")
add_library(akasset STATIC ${AK_ASSET_SOURCE})

# ################### #
# # Compile Options # #
# ################### #
set_target_properties(akasset PROPERTIES CXX_EXTENSIONS OFF CXX_STANDARD_REQUIRED ON CXX_STANDARD 17)
target_compile_options(akasset PRIVATE ${AK_BUILD_CXX_WARNING_FLAGS})
target_compile_options(akasset PRIVATE ${AK_BUILD_CXX_COMPILER_FLAGS})

# ##################### #
# # Internal Includes # #
# ##################### #
target_include_directories(akasset PUBLIC  inc)
target_include_directories(akasset PRIVATE src)

target_include_directories(akasset SYSTEM PUBLIC ${AK_ENGINE_INCLUDE_DIR})
target_include_directories(akasset SYSTEM PUBLIC ${AK_RENDER_INCLUDE_DIR})

target_link_libraries(akasset PUBLIC akengine)
target_link_libraries(akasset PUBLIC akrender)

# ##################### #
# # External Includes # #
# ##################### #
target_include_directories(akasset SYSTEM PUBLIC ${RAPIDJSON_INCLUDE_DIR})
target_include_directories(akasset SYSTEM PUBLIC ${STB_INCLUDE_DIR})


