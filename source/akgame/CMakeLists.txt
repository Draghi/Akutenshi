cmake_minimum_required(VERSION 3.2) 

if(NOT DEFINED AK_BUILD_ENV_SETUP)
	include("cmake/SetupBuildEnvironment.cmake")
endif()

project(akgame VERSION 1.0.0.0)
sugar_include("src")
add_library(akgame STATIC ${AK_GAME_SOURCE})

# ################### #
# # Compile Options # #
# ################### #
set_target_properties(akgame PROPERTIES CXX_EXTENSIONS OFF CXX_STANDARD_REQUIRED ON CXX_STANDARD 17)
target_compile_options(akgame PRIVATE ${AK_BUILD_CXX_WARNING_FLAGS})
target_compile_options(akgame PRIVATE ${AK_BUILD_CXX_COMPILER_FLAGS})

# ##################### #
# # Internal Includes # #
# ##################### #
target_include_directories(akgame PUBLIC  inc)
target_include_directories(akgame PRIVATE src)

target_include_directories(akgame SYSTEM PUBLIC ${AK_ASSET_INCLUDE_DIR})
target_include_directories(akgame SYSTEM PUBLIC ${AK_ENGINE_INCLUDE_DIR})
target_include_directories(akgame SYSTEM PUBLIC ${AK_INPUT_INCLUDE_DIR})
target_include_directories(akgame SYSTEM PUBLIC ${AK_RENDER_INCLUDE_DIR})
target_include_directories(akgame SYSTEM PUBLIC ${AK_SOUND_INCLUDE_DIR})

target_link_libraries(akgame PUBLIC akasset)
target_link_libraries(akgame PUBLIC akengine)
target_link_libraries(akgame PUBLIC akinput)
target_link_libraries(akgame PUBLIC akrender)
target_link_libraries(akgame PUBLIC aksound)

# ##################### #
# # External Includes # #
# ##################### #
