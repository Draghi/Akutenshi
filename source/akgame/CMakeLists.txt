cmake_minimum_required(VERSION 3.2) 
project(akgame VERSION 1.0.0.0)

if(NOT DEFINED AK_BUILD_ENV_SETUP)
	include("cmake/SetupBuildEnvironment.cmake")
endif()

sugar_include("src")
add_library(akgame STATIC ${AK_GAME_SOURCE})

# ################### #
# # Compile Options # #
# ################### #
target_compile_options(akgame PRIVATE ${AK_BUILD_CXX_WARNING_FLAGS})
target_compile_options(akgame PRIVATE ${AK_BUILD_CXX_COMPILER_FLAGS})
set_target_properties(akgame PROPERTIES CXX_EXTENSIONS OFF CXX_STANDARD_REQUIRED ON CXX_STANDARD 17)

# ##################### #
# # Internal Includes # #
# ##################### #
target_include_directories(akgame PUBLIC  inc)
target_include_directories(akgame PRIVATE src)

target_include_directories(akgame SYSTEM PUBLIC ${AK_ASSET_INCLUDE_DIR})
target_include_directories(akgame SYSTEM PUBLIC ${AK_COMMON_INCLUDE_DIR})
target_include_directories(akgame SYSTEM PUBLIC ${AK_ENGINE_INCLUDE_DIR})
target_include_directories(akgame SYSTEM PUBLIC ${AK_GAME_INCLUDE_DIR})
target_include_directories(akgame SYSTEM PUBLIC ${AK_INPUT_INCLUDE_DIR})
target_include_directories(akgame SYSTEM PUBLIC ${AK_MATH_INCLUDE_DIR})
target_include_directories(akgame SYSTEM PUBLIC ${AK_RENDER_INCLUDE_DIR})
target_include_directories(akgame SYSTEM PUBLIC ${AK_SOUND_INCLUDE_DIR})

target_link_libraries(akgame akasset)
target_link_libraries(akgame akcommon)
target_link_libraries(akgame akengine)
target_link_libraries(akgame akinput)
target_link_libraries(akgame akmath)
target_link_libraries(akgame akrender)
target_link_libraries(akgame aksound)

# ##################### #
# # External Includes # #
# ##################### #
